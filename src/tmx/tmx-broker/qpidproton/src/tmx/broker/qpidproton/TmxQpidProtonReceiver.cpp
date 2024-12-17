/*!
 * Copyright (c) 2024 Battelle Memorial Institute
 *
 * All Rights Reserved.
 *
 * @file TmxQpidProtonReceiver.cpp
 *
 *  Created on: Apr 25, 2024
 *      @author: Gregory M. Baumgardner
 */

#include <tmx/broker/qpidproton/TmxQpidProtonClient.hpp>

#include <tmx/common/TmxLogger.hpp>
#include <tmx/common/TmxTypeRegistry.hpp>
#include <tmx/message/TmxMessage.hpp>

#include <proton/connection_options.hpp>
#include <proton/container.hpp>
#include <proton/delivery.hpp>
#include <proton/message.hpp>
#include <proton/receiver.hpp>
#include <proton/receiver_options.hpp>
#include <proton/source.hpp>
#include <proton/work_queue.hpp>

#include <thread>
#include <proton/connection_options.hpp>

using namespace tmx::common;
using namespace tmx::message;

namespace tmx {
namespace broker {
namespace qpidproton {

static typename types::Properties_::key_t _credit { "credit" };

typedef typename common::TmxFunctor<common::types::Any const &, message::TmxMessage const &>::type::type cb_type;

class TmxQpidProtonReceiver: public TmxQpidProtonClient, public proton::messaging_handler {
public:
    TmxQpidProtonReceiver(TmxBrokerContext &ctx): context(ctx) { }
    void on_message(proton::delivery &, proton::message &) override;
    void on_receiver_open(proton::receiver &) override;
    void on_receiver_detach(proton::receiver &) override;
    void on_receiver_close(proton::receiver &) override;
    void on_receiver_error(proton::receiver &) override;

    TmxBrokerContext &context;
    proton::receiver *receiver = nullptr;
};

static std::list<TmxQpidProtonReceiver> _receivers;
static std::mutex _recv_lock;

void TmxQpidProtonClient::subscribe(TmxBrokerContext &ctx, const_string topicName, TmxTypeDescriptor const &cb) noexcept {
    if (!cb) {
        std::string err { "Callback " };
        err.append(cb.get_type_name());
        err.append(" is not valid.");

        this->on_subscribed(ctx, { EINVAL, err }, topicName, cb);
        return;
    }

    auto callback = cb.as_instance<cb_type>();
    if (!callback) {
        std::string err { "Callback " };
        err.append(cb.get_type_name());
        err.append(" is not the correct signature. Expecting ");
        err.append(type_fqname<cb_type>());

        this->on_subscribed(ctx, { EBADRQC, err }, topicName, cb);
        return;
    }

    std::shared_ptr<TmxQpidProtonConnection> conn;

    if (ctx.count("connection"))
        conn = std::const_pointer_cast<TmxQpidProtonConnection>(types::as<TmxQpidProtonConnection>(ctx.at("connection")));

    if (!conn) {
        this->on_connected(ctx, { EINVAL, "Context " + ctx.get_id() + " was not initialized properly"});
        return;
    }

    // Register the handler
    callback_registry(ctx.get_id(), topicName.data()).register_handler(*callback, cb.get_typeid(), cb.get_type_short_name());

    // AMPQ topic names have a dot separator instead of slash
    typename types::Properties_::key_t topic { topicName.data() };
    std::replace(topic.begin(), topic.end(), std::filesystem::path::preferred_separator, '.');

    // Address names for Service Bus include subscriptions
    std::string address = topic;
    const TmxData params { ctx.get_parameters() };
    if (params["subscription"]) {
        address.append("/Subscriptions/");
        address.append(params["subscription"].to_string().c_str());
    }

    std::lock_guard<std::mutex> lock(_recv_lock);
    _receivers.emplace_back(ctx);

    auto recOpts = conn->container().receiver_options();
    recOpts.handler(_receivers.back());
    recOpts.name(topic);

    // Open a new receiver. This gets an associated connection
    conn->container().open_receiver(conn->url() + "/" + address,
        recOpts, conn->container().client_connection_options());

    // The asynchronous callback will determine the success of the subscription
    std::this_thread::yield();
}

void TmxQpidProtonReceiver::on_receiver_open(proton::receiver &receiver) {
    TLOG(DEBUG3) << "Enter " << TMX_PRETTY_FUNCTION << " for " << receiver.container().id();

    // Save the receiver
    this->receiver = &receiver;

    std::string msg { "Successfully opened receiver " };
    msg.append(receiver.name());
    msg.append(" on topic ");
    msg.append(receiver.source().address());
    msg.append(" with connection ");
    msg.append(receiver.container().id());

    std::lock_guard<std::mutex> lock(this->context.get_thread_lock());
    this->on_subscribed(this->context, { EXIT_SUCCESS, msg }, receiver.source().address(),
            this->callback_registry(receiver.container().id(), receiver.source().address()).get(receiver.name()));
}

void TmxQpidProtonClient::unsubscribe(TmxBrokerContext &ctx, const_string topic, TmxTypeDescriptor const &cb) noexcept {
    // Remove the handler since it no longer will be used
    this->callback_registry(ctx.get_id(), topic.data()).unregister(cb.get_typeid());

    // Close the associated receiver
    std::lock_guard<std::mutex> lock(_recv_lock);
    for (auto &recv: _receivers) {
        if (recv.receiver) {
            if (topic == TmxTypeRegistry(recv.receiver->name()).get_namespace()) {
                recv.receiver->work_queue().add([receiver = recv.receiver]() {
                    if (receiver)
                        receiver->close();
                });
            }
        }
    }

    // The asynchronous callback will determine the success of the destruction
    std::this_thread::yield();
}

void TmxQpidProtonReceiver::on_receiver_close(proton::receiver &receiver) {
    TLOG(DEBUG3) << "Enter " << TMX_PRETTY_FUNCTION << " for " << receiver.container().id();

    std::lock_guard<std::mutex> lock(_recv_lock);
    this->receiver = nullptr;

    auto ptr = &receiver;
    _receivers.remove_if([ptr](auto const &rec) {
        return (ptr == rec.receiver);
    });

    std::lock_guard<std::mutex> c_lock(this->context.get_thread_lock());
    this->on_unsubscribed(this->context, to_error(receiver.error()), receiver.source().address(),
        this->callback_registry(receiver.container().id(), receiver.source().address()).get(receiver.name()));

    receiver.connection().close();
}

void TmxQpidProtonReceiver::on_receiver_error(proton::receiver &receiver) {
    TLOG(DEBUG3) << "Enter " << TMX_PRETTY_FUNCTION << " for " << receiver.container().id();
    receiver.close();
}

void TmxQpidProtonReceiver::on_receiver_detach(proton::receiver &r) {
    TLOG(DEBUG3) << "Enter " << TMX_PRETTY_FUNCTION << " for " << r.container().id();
}

void TmxQpidProtonReceiver::on_message(proton::delivery &d, proton::message &m) {
    if (!d.settled())
        d.accept();

    types::Any _id;
    _id.emplace<std::string>(d.receiver().container().id());

    // Convert dots back to slashes
    TmxMessage msg;
    msg.set_id(m.content_type());
    msg.set_topic(TmxTypeRegistry(d.receiver().source().address()).get_namespace().data());

    // Remove the subscription from the topic
    auto index = msg.get_topic().find("/Subscription");
    if (index != std::string::npos)
        msg.set_topic(msg.get_topic().substr(0, index));

    msg.set_source(m.subject());
    msg.set_encoding(m.content_encoding());
    msg.set_timepoint(std::chrono::system_clock::time_point(std::chrono::milliseconds(m.creation_time().milliseconds())));
    msg.set_payload(proton::coerce<std::string>(m.body()));

    types::UInt<TMX_METADATA_QOS_BITS> _qos;
    types::UInt<TMX_METADATA_PRIORITY_BITS> _priority;
    types::UInt<TMX_METADATA_BASE_BITS> _base;
    types::unpack(m.priority(), _qos, _priority, _base);
    msg.set_QoS(_qos);
    msg.set_priority(_priority);
    msg.set_base(_base);

    types::UInt<TMX_METADATA_ASSIGNMENT_GROUP_BITS> _grp;
    types::UInt<TMX_METADATA_ASSIGNMENT_ID_BITS> _aid;
    types::unpack(m.group_sequence(), _grp, _aid);
    msg.set_assignment_group(_grp);
    msg.set_assignment_id(_aid);

    this->callback(d.receiver().container().id(), msg);
}

} /* End namespace qpidproton */
} /* End namespace broker */
} /* End namspace tmx */