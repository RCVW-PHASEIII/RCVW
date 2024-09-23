/*!
 * Copyright (c) 2021 Battelle Memorial Institute
 *
 * All Rights Reserved.
 *
 * @file KafkaBroker.cpp
 *
 *  Created on: Aug 30, 2021
 *      @author: gmb
 */

#include <tmx/broker/apache/kafka/TmxKafkaBroker.hpp>

#include <tmx/common/TmxLogger.hpp>
#include <tmx/common/types/Any.hpp>
#include <tmx/common/TmxTypeRegistrar.hpp>
#include <tmx/message/TmxData.hpp>
#include <tmx/message/codec/serializer/TmxDataSerializer.hpp>

#include <atomic>
#include <cerrno>
#include <cstring>
#include <librdkafka/rdkafkacpp.h>
#include <memory>
#include <sstream>
#include <thread>

using namespace tmx::broker;
using namespace tmx::common;
using namespace tmx::message;
using namespace tmx::message::codec::serializer;

namespace tmx {
namespace broker {
namespace apache {
namespace kafka {

typedef typename tmx::common::TmxFunctor<types::Any const &, TmxMessage const &>::type::type cb_type;

static TmxBrokerContext _errCtx { "kafka://localhost" };

template <typename _T>
auto _key() {
    static const typename types::Properties_::key_t _key{ type_short_name<_T>().data() };
    return _key;
}

template <typename _T, typename _V = _T>
std::shared_ptr<_V> _get(TmxBrokerContext &ctx) {
    if (ctx.count(_key<_T>()))
        return types::as<_V>(ctx.at(_key<_T>()));

    return { };
}

template <typename _T, typename _V>
std::shared_ptr<_V> _put(_V *ptr, TmxBrokerContext &ctx) {
    return ctx[_key<_T>()].template emplace<std::shared_ptr<_V> >(ptr);
}

TmxKafkaBroker::TmxKafkaBroker() noexcept {
    this->register_broker("kafka");
}

TmxTypeDescriptor TmxKafkaBroker::get_descriptor() const noexcept {
    static const auto &_desc = TmxBrokerClient::get_descriptor();
    return { _desc.get_instance(), typeid(TmxKafkaBroker), type_fqname(*this).data() };
}

types::Any TmxKafkaBroker::get_broker_info(TmxBrokerContext &ctx) const noexcept {
    message::TmxData data { TmxBrokerClient::get_broker_info(ctx) };

    const TmxData params { ctx.get_parameters() };
    data["config"]["global"] = params["global"];
    data["config"]["consumer-group"] = params["consumer-group"];

    if (this->is_connected(ctx)) {
        auto consumer = _get<RdKafka::KafkaConsumer>(ctx);
        if (consumer) {
            RdKafka::Metadata *md = nullptr;
            auto err = consumer->metadata(true, nullptr, &md, 5000);

            if (err) {
                typedef common::types::Properties<common::types::Any> props_type;
                const TmxError tmxError { err, RdKafka::err2str(err) };

                TLOG(ERR) << "Unable to connect to Kafka broker: " << tmxError.get_message();
                data["topics"].get_container().emplace<props_type>(tmxError);
                data["brokers"].get_container().emplace<props_type>(tmxError);
            } else {
                if (md && md->topics()) {
                    std::size_t i = 0;
                    for (auto tp: *(md->topics())) {
                        if (tp && tp->partitions()) {
                            // Ignore internal Kafka topics
                            if (tp->topic().substr(0, 2) == "__")
                                continue;

                            for (auto partition: *(tp->partitions())) {
                                if (partition)
                                    data["topics"][i]["paritions"][std::to_string(partition->id())]["leader"] = partition->leader();
                            }

                            std::string nm{ tp->topic() };
                            std::replace(nm.begin(), nm.end(), '.', std::filesystem::path::preferred_separator);
                            data["topics"][i++]["name"] = nm;
                        }
                    }
                }

                if (md && md->brokers()) {
                    std::size_t i = 0;
                    for (auto br: *(md->brokers())) {
                        if (br) {
                            data["brokers"][i]["port"] = br->port();
                            data["brokers"][i]["host"] = std::string(br->host());
                            data["brokers"][i]["id"] = br->id();

                            i++;
                        }
                    }
                }
            }

            delete md;
            md = nullptr;

            data["cluster-id"] = consumer->clusterid(0);
            data["controller-id"] = consumer->controllerid(0);
        }
    }

    return data.get_container();
}

void TmxKafkaBroker::initialize(TmxBrokerContext &ctx) noexcept {
    if (ctx.get_state() != TmxBrokerState::uninitialized) return;
    if (std::strcmp("kafka", ctx.get_scheme())) return;

    // Store the Kafka context
    auto config = _put<RdKafka::Conf>(RdKafka::Conf::create(RdKafka::Conf::CONF_GLOBAL), ctx);
    if (!config) {
        this->on_initialized(ctx, { -1, "Unable to create global configuration" });
        return;
    }

    std::string _errstr;
    auto ec = config->set("log.thread.name", "true", _errstr);
    if (ec != RdKafka::Conf::CONF_OK) {
        this->on_initialized(ctx, { ec, _errstr });
        return;
    }

    ec = config->set("client.id", ctx.get_id().c_str(), _errstr);
    if (ec != RdKafka::Conf::CONF_OK) {
        this->on_initialized(ctx, { ec, _errstr });
        return;
    }

    ec = config->set("dr_cb", (RdKafka::DeliveryReportCb *)this, _errstr);
    if (ec != RdKafka::Conf::CONF_OK) {
        this->on_initialized(ctx, { ec, _errstr });
        return;
    }

    ec = config->set("event_cb", (RdKafka::EventCb *)this, _errstr);
    if (ec != RdKafka::Conf::CONF_OK) {
        this->on_initialized(ctx, { ec, _errstr });
        return;
    }

    ec = config->set("rebalance_cb", (RdKafka::RebalanceCb *)this, _errstr);
    if (ec != RdKafka::Conf::CONF_OK) {
        this->on_initialized(ctx, { ec, _errstr });
        return;
    }

    // Additional Kafka settings supplied by the context config
    TmxData data { ctx.get_parameters() };

    // Make sure the bootstrap server is loaded
    std::string servers = ctx.get_host() + (ctx.get_port().length() ? ":" + ctx.get_port() : "");
    for (auto &srv: data["bootstrap.servers"].to_array())
        servers += "," + TmxData(srv).to_string();

    data["global"]["bootstrap.servers"] = servers;

    // Make sure the default consists of...
    // ...auto-creation of topics
    if (!data["global"]["allow.auto.create.topics"])
        data["global"]["allow.auto.create.topics"] = true;

    // ...low latency
    if (!data["global"]["linger.ms"])
        data["global"]["linger.ms"] = 0;

    for (auto const &kv: data["global"].to_map()) {
        TLOG(DEBUG) << "Setting Global Kafka config " << kv.first << "=" << kv.second;
        ec = config->set(kv.first, TmxData(kv.second).to_string(), _errstr);
        if (ec != RdKafka::Conf::CONF_OK) {
            this->on_initialized(ctx, { ec, _errstr });
            return;
        }
    }

    // Make sure there is a default consumer group
    data["consumer-group"] = std::string(ctx.get_id());

    // TODO: Set other default parameters from Context
    TmxBrokerClient::initialize(ctx);
}

void TmxKafkaBroker::destroy(tmx::broker::TmxBrokerContext &ctx) noexcept {
    if (this->is_connected(ctx))
        this->disconnect(ctx);

    // Delete the remaining Kafka resources
    ctx.erase(_key<RdKafka::Producer>());
    ctx.erase(_key<RdKafka::Conf>());

    RdKafka::wait_destroyed(5000);
}

void TmxKafkaBroker::disconnect(TmxBrokerContext &ctx) noexcept {
    TmxBrokerClient::disconnect(ctx);

    // Flush the producer
    auto producer = _get<RdKafka::Producer>(ctx);
    if (producer) producer->flush(2500);

    auto recvThread = _get<std::thread>(ctx);
    if (recvThread && recvThread->joinable())
        recvThread->join();

    ctx.erase(_key<typename decltype(recvThread)::element_type>());

    // The broker does not produce in a disconnected state anyway, so leave the producer
    // However, the consumer should be removed so it can be recreated, potentially as part
    // of a different consumer group.
    auto consumer = _get<RdKafka::KafkaConsumer>(ctx);
    if (consumer) consumer->close();
    ctx.erase(_key<typename decltype(consumer)::element_type>());
}

void TmxKafkaBroker::connect(TmxBrokerContext &ctx, const types::Any &params) noexcept {
    if (ctx.get_state() != TmxBrokerState::initialized && ctx.get_state() != TmxBrokerState::disconnected)
        return;

    auto config = _get<RdKafka::Conf>(ctx);
    if (!config) {
          this->on_connected(ctx, { 1, "Broker context " + ctx.to_string() + " not initialized properly." });
        return;
    }

    std::string _errstr;

    // Create a producer to use for this connection
    auto producer = _get<RdKafka::Producer>(ctx);
    if (!producer) {
        producer = _put<RdKafka::Producer>(RdKafka::Producer::create(config.get(), _errstr), ctx);
        if (!producer) {
            this->on_connected(ctx, { 2, _errstr });
            return;
        }
    }

    // Create a consumer to use for this connection
    std::string group = TmxData(ctx.get_parameters())["consumer-group"].to_string();

    const TmxData p { params };
    if (p["consumer-group"] && p["consumer-group"].to_string().length())
        group = p["consumer-group"].to_string();

    auto ec = config->set("group.id", group.c_str(), _errstr);
    if (ec != RdKafka::Conf::CONF_OK) {
        this->on_connected(ctx, { ec, _errstr });
        return;
    }

    auto consumer = _get<RdKafka::KafkaConsumer>(ctx);
    if (!consumer) {
        consumer = _put<RdKafka::KafkaConsumer>(RdKafka::KafkaConsumer::create(config.get(), _errstr), ctx);
        if (!consumer) {
            this->on_connected(ctx, { 3, _errstr });
            return;
        }
    }

    TmxBrokerClient::connect(ctx, params);
}

void TmxKafkaBroker::publish(TmxBrokerContext &ctx, const message::TmxMessage &msg) noexcept {
    TLOG(DEBUG3) << "Enter " << TMX_PRETTY_FUNCTION;

    if (!this->is_connected(ctx)) {
        this->on_published(ctx, { 1, "Broker context " + ctx.to_string() + " is not connected."}, msg);
        return;
    }

    // The topic names cannot have slashes in Apache. Convert to dots
    auto topic = std::filesystem::path(msg.get_topic().data()).native();
    std::replace(topic.begin(), topic.end(), std::filesystem::path::preferred_separator, '.');

    auto producer = _get<RdKafka::Producer>(ctx);
    if (!producer) {
        this->on_published(ctx, { 2, "Broker context " + ctx.to_string() + " is missing producer handle." }, msg);
        return;
    }

    auto headers = RdKafka::Headers::create();
    if (!headers) {
        this->on_published(ctx, { 3, "Could not create headers for message" }, msg);
        return;
    }

    headers->add("content-type", msg.get_id());
    headers->add("content-source", msg.get_source());
    headers->add("content-encoding", msg.get_encoding());

    TLOG(DEBUG2) << "Producing " << msg.get_length() << " bytes to topic " << topic << ": " << msg.get_payload_string();

    auto tm = std::chrono::duration_cast<std::chrono::milliseconds>(msg.get_timepoint().time_since_epoch());
    auto ec = producer->produce(topic, RdKafka::Topic::PARTITION_UA, RdKafka::Producer::RK_MSG_COPY,
                                (void *)msg.get_payload().data(), msg.get_length(), nullptr, 0,
                                tm.count(), headers, (void *)&ctx);
    if (ec != RdKafka::ERR_NO_ERROR) {
        this->on_published(ctx, { ec, RdKafka::err2str(ec) }, msg);
        return;
    }

    // Poll the producer to check delivery
    producer->poll(0);
}

void TmxKafkaBroker::subscribe(TmxBrokerContext &ctx, const_string topicName, const TmxTypeDescriptor &cb) noexcept {
    if (!this->is_connected(ctx)) {
        this->on_subscribed(ctx, { 1, "Broker context " + ctx.to_string() + " is not connected."}, topicName, cb);
        return;
    }

    auto consumer = _get<RdKafka::KafkaConsumer>(ctx);
    if (!consumer) {
        this->on_subscribed(ctx, { 2, "Broker context " + ctx.to_string() + " is missing consumer handle."},
                            topicName, cb);
        return;
    }

    if (!cb) {
        std::string err { "Callback " };
        err.append(cb.get_type_name());
        err.append(" is not valid.");
        this->on_subscribed(ctx, { 4, err }, topicName, cb);
        return;
    }

    auto callback = cb.as_instance<cb_type>();

    if (!callback) {
        std::string err { "Callback " };
        err.append(cb.get_type_name());
        err.append(" is not the correct signature. Expecting ");
        err.append(type_fqname<cb_type>());
        this->on_subscribed(ctx, { 5, err }, topicName, cb);
        return;
    }

    // Register the handler
    std::string nm { topicName.data() };
    callback_registry(ctx.get_id(), nm).register_handler(*callback, cb.get_typeid(), cb.get_type_short_name());

    // The topic names cannot have slashes in Apache. Convert to dots
    std::replace(nm.begin(), nm.end(), std::filesystem::path::preferred_separator, '.');

    // See if the topic was already subscribed to
    std::vector<std::string> topics;
    auto ec = consumer->subscription(topics);
    if (ec) {
        this->on_subscribed(ctx, { ec, RdKafka::err2str(ec) }, topicName, cb);
        return;
    }

    auto iter = std::find(topics.begin(), topics.end(), nm);
    if (iter == topics.end()) {
        // Create the topic
        std::string _errstr;
        if (!RdKafka::Topic::create(consumer.get(), nm, nullptr, _errstr)) {
            this->on_subscribed(ctx, { 4, _errstr }, topicName, cb);
            return;
        }

        topics.emplace_back(nm);
        ec = consumer->subscribe(topics);
        if (ec) {
            this->on_subscribed(ctx, { ec, RdKafka::err2str(ec) }, topicName, cb);
            return;
        }
    }

    // Start up the consume thread upon the first subscription
    if (!_get<std::thread>(ctx)) {
        _put<std::thread>(new std::thread([this, &ctx]() {
            std::size_t cnt = 0;

            while (this->is_connected(ctx)) {
                auto consumer = _get<RdKafka::KafkaConsumer>(ctx);
                if (consumer) {
                    RdKafka::Message *msg = consumer->consume(1);
                    if (msg) {
                        // Check for error
                        if (msg->err()) {
                            if (msg->err() != RdKafka::ErrorCode::ERR__TIMED_OUT)
                                this->on_error(ctx, { msg->err(), msg->errstr() });
                        } else {
                            this->consume_cb(*msg, &ctx);
                        }

                        delete msg;
                    }
                }

                std::this_thread::sleep_for(std::chrono::microseconds(10));
            }
        }), ctx);
    }

    TmxBrokerClient::subscribe(ctx, topicName, cb);
}

void TmxKafkaBroker::unsubscribe(TmxBrokerContext &ctx, const_string topicName, const TmxTypeDescriptor &cb) noexcept {
    // Unregister the handler
    std::string nm { topicName.data() };
    callback_registry(ctx.get_id(), nm).unregister(cb.get_typeid());

    // The topic names cannot have slashes in Apache. Convert to dots
    std::replace(nm.begin(), nm.end(), std::filesystem::path::preferred_separator, '.');

    auto consumer = _get<RdKafka::KafkaConsumer>(ctx);
    if (consumer) {
        std::vector<std::string> topics;
        auto ec = consumer->subscription(topics);
        if (ec) {
            this->on_unsubscribed(ctx, { ec, RdKafka::err2str(ec) }, topicName, cb);
            return;
        }

        auto iter = std::find(topics.begin(), topics.end(), nm);
        if (iter != topics.end()) {
            topics.erase(iter);

            ec = consumer->subscribe(topics);
            if (ec) {
                this->on_unsubscribed(ctx, { ec, RdKafka::err2str(ec) }, topicName, cb);
                return;
            }

            // Disconnect this context and remove the consumer once all the topic subscriptions are removed
            if (topics.size() == 0)
                this->disconnect(ctx);
        }
    }

    TmxBrokerClient::unsubscribe(ctx, topicName, cb);
}


void TmxKafkaBroker::event_cb(RdKafka::Event &event) {
    TmxError _err { event.err(), err2str(event.err()) };

	switch(event.type()) {
        case RdKafka::Event::Type::EVENT_ERROR:
            TLOG(ERROR) << err2str(event.err());
            this->on_error(_errCtx, _err, event.fatal());
            break;
        case RdKafka::Event::Type::EVENT_LOG:
            // Only the debug level differs
            auto lvl = enums::enum_cast<TmxLogLevel>(enums::enum_integer(event.severity()));
            if (event.severity() == RdKafka::Event::Severity::EVENT_SEVERITY_DEBUG)
                lvl = TmxLogLevel::DEBUG;

            if (lvl && TmxLogger::can_log(lvl.value()))
                TMX_LOGGER(lvl.value()) << event.str();
            else
                TLOG(DEBUG2) << event.str();

            break;
	}
}

message::TmxMessage convert(RdKafka::Message &message) {
    message::TmxMessage msg;
    msg.set_timepoint(std::chrono::system_clock::time_point(std::chrono::milliseconds(message.timestamp().timestamp)));

    // Convert dots back to slashes
    typename types::Properties_::key_t nm { message.topic_name() };
    std::replace(nm.begin(), nm.end(), '.', std::filesystem::path::preferred_separator);
    msg.set_topic(nm);

    msg.set_payload((const char *)message.payload(), message.len());
    if (message.headers()) {
        for (auto header: message.headers()->get("content-type"))
            msg.get_id().assign((const char *)header.value(), header.value_size());
        for (auto header: message.headers()->get("content-source"))
            msg.get_source().assign((const char *)header.value(), header.value_size());
        for (auto header: message.headers()->get("content-encoding"))
            msg.get_encoding().assign((const char *)header.value(), header.value_size());
    }

    return { msg };
}

void TmxKafkaBroker::dr_cb(RdKafka::Message &message) {
	TmxBrokerContext *ctx = (TmxBrokerContext *)message.msg_opaque();
	if (ctx) {
        auto data = to_byte_sequence((byte_t *)message.payload(), message.len());
        this->on_published(*ctx, { message.err(), message.errstr() }, convert(message));
	}
}

void TmxKafkaBroker::consume_cb(RdKafka::Message &message, void *opaque) {
    TmxBrokerContext *ctx = (TmxBrokerContext *)opaque;
    auto data = to_byte_sequence((byte_t *)message.payload(), message.len());

    TLOG(DEBUG2) << this->get_descriptor().get_type_name() << ": Received on channel "
                 << ctx->get_id() << ": " << data;

    if (ctx && data.length())
        this->callback(ctx->get_id(), convert(message));
}

void TmxKafkaBroker::rebalance_cb(RdKafka::KafkaConsumer *consumer, RdKafka::ErrorCode ec,
                                  std::vector<RdKafka::TopicPartition *> &partitions) {
    if (TmxLogger::can_log(TmxLogLevel::DEBUG3)) {
        // Print the partitions
        for (std::size_t i = 0; i < partitions.size(); i++) {
            if (partitions[i])
                TLOG(DEBUG3) << partitions[i]->topic() << "[" << partitions[i]->partition() << "]";
        }
    }

    RdKafka::Error *error = nullptr;
    RdKafka::ErrorCode ret_err = RdKafka::ERR_NO_ERROR;

    if (ec == RdKafka::ERR__ASSIGN_PARTITIONS) {
        if (consumer->rebalance_protocol() == "COOPERATIVE")
            error = consumer->incremental_assign(partitions);
        else
            ret_err = consumer->assign(partitions);
    } else {
        if (consumer->rebalance_protocol() == "COOPERATIVE") {
            error = consumer->incremental_unassign(partitions);
        } else {
            ret_err = consumer->unassign();
        }
    }

    if (error) {
        this->on_error(_errCtx, { error->code(), error->str() }, false);
        delete error;
    } else if (ret_err) {
       this->on_error(_errCtx, { ret_err, RdKafka::err2str(ret_err) }, false);
    }
}

static common::TmxTypeRegistrar<TmxKafkaBroker> _kafka_broker;

} /* namespace kafka */
} /* namespace apache */
} /* namespace broker */
} /* namespace tmx */
