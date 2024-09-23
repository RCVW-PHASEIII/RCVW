/*!
 * Copyright (c) 2024 Battelle Memorial Institute
 *
 * All Rights Reserved.
 *
 * @file TmxQpidProtonSender.cpp
 *
 *  Created on: Apr 25, 2024
 *      @author: Gregory M. Baumgardner
 */

#include <tmx/broker/qpidproton/TmxQpidProtonClient.hpp>

#include <tmx/broker/TmxBrokerClient.hpp>
#include <tmx/common/TmxLogger.hpp>
#include <tmx/common/TmxTypeRegistry.hpp>
#include <tmx/message/TmxMessage.hpp>

#include <proton/connection.hpp>
#include <proton/container.hpp>
#include <proton/duration.hpp>
#include <proton/message.hpp>
#include <proton/message_id.hpp>
#include <proton/sender.hpp>
#include <proton/sender_options.hpp>
#include <proton/target.hpp>
#include <proton/tracker.hpp>
#include <proton/work_queue.hpp>

#include <queue>
#include <thread>

using namespace tmx::common;
using namespace tmx::message;

namespace tmx {
namespace broker {
namespace qpidproton {

void TmxQpidProtonClient::publish(TmxBrokerContext &ctx, TmxMessage const &msg) noexcept {
    // Make a copy of the message
    std::shared_ptr<TmxMessage> message;
    try {
        message = std::make_shared<TmxMessage>(msg);
    } catch (std::exception &ex) {
        this->on_published(ctx, { ex }, msg);
        return;
    }

    if (!this->is_connected(ctx)) {
        std::string err{ "No connection established to " };
        err.append(ctx.to_string());
        err.append(". Current state is ");
        err.append(enums::enum_name(ctx.get_state()));
        err.append(".");

        this->on_published(ctx, { ENOTCONN, err }, *message);
        return;
    }

    this->get_container(ctx).schedule(proton::duration(), [this, &ctx, message]() -> void {
         auto &connection = this->get_connection(ctx);

        // Create a new sender for this topic and attach the message
        connection.work_queue().add([&connection, message]() {
            // AMPQ topic names have a dot separator instead of slash
            std::string topic { message->get_topic().c_str() };
            std::replace(topic.begin(), topic.end(), std::filesystem::path::preferred_separator, '.');

            connection.open_sender(topic, connection.container().sender_options()).user_data(
                    new decltype(message)(message));
        });
    });

    // Results will be determined asynchronously
    std::this_thread::yield();
}

void TmxQpidProtonClient::on_published(TmxBrokerContext &ctx, common::TmxError const &err,
                                       message::TmxMessage const &msg) noexcept {
    TmxBrokerClient::on_published(ctx, err, msg);

    // See if the message failed due to a timeout waiting for a connection
    // Try again if the quality of service is greater than 0
    if (msg.get_QoS() && err.get_code() == ENOTCONN) {
        const TmxData params { ctx.get_parameters() };

        // Try again up to a certain number of times
        int max = 5;
        if (params["reconnect-max-attempts"])
            max = params["reconnect-max-attempts"];

        TLOG(INFO) << "Metadata: " << msg.get_metadata();

        if (msg.get_attempt() > max) {
            std::string err { "Dropping message to topic " };
            err.append(msg.get_topic());
            err.append(" with connection ");
            err.append(ctx.get_id());
            err.append(" after ");
            err.append(std::to_string(max));
            err.append(" attempts.");

            TmxBrokerClient::on_published(ctx, { ETIME, err }, msg);
            std::this_thread::yield();
        } else {
            TmxMessage tmp { msg };
            tmp.set_attempt(msg.get_attempt() + 1);
            this->publish(ctx, tmp);
        }
    }
}

void TmxQpidProtonClient::on_sender_open(proton::sender &s) {
    TLOG(DEBUG3) << "Enter " << TMX_PRETTY_FUNCTION << " for " << s.container().id();
}

void TmxQpidProtonClient::on_sender_close(proton::sender &s) {
    TLOG(DEBUG3) << "Enter " << TMX_PRETTY_FUNCTION << " for " << s.container().id();
}

void TmxQpidProtonClient::on_sender_detach(proton::sender &s) {
    TLOG(DEBUG3) << "Enter " << TMX_PRETTY_FUNCTION << " for " << s.container().id();
}

void TmxQpidProtonClient::on_sender_error(proton::sender &s) {
    TLOG(DEBUG3) << "Enter " << TMX_PRETTY_FUNCTION << " for " << s.container().id();

    // We at least know the topic
    TmxMessage tmp;
    tmp.set_topic(TmxTypeRegistry(s.target().address()).get_namespace().data());

    this->on_published(this->get_context(s.container().id()), this->to_error(s.error()), tmp);
}

void TmxQpidProtonClient::on_sendable(proton::sender &s) {
    TLOG(DEBUG3) << "Enter " << TMX_PRETTY_FUNCTION << " for " << s.container().id();

    // See if there is a message to send
    auto msg = (std::shared_ptr<TmxMessage> *)s.user_data();
    s.user_data(nullptr);

    if (msg && msg->get()) {
        auto const &message = *(msg->get());

        if (!s.credit()) {
            std::string err{ "Dropping message with no credits for sender " };
            err.append(s.name());
            err.append(" on topic ");
            err.append(message.get_topic());
            err.append(" with connection ");
            err.append(s.container().id());

            this->on_published(this->get_context(s.container()), { EBUSY, err }, message);
            return;
        }

        TLOG(DEBUG2) << "Writing " << message.get_length() << " bytes to broker topic "
                     << message.get_topic() << " on sender " << s.name();

        // Build a Qpid Proton message from the TMX message
        proton::message m;
        m.address(s.target().address());
        m.content_type(message.get_id());
        m.subject(message.get_source());
        m.content_encoding(message.get_encoding());
        m.body(message.get_payload_string());

        proton::timestamp ts{
                std::chrono::duration_cast<std::chrono::milliseconds>(
                        message.get_timepoint().time_since_epoch()).count()
        };
        m.creation_time(ts);

        types::UInt<TMX_METADATA_QOS_BITS> _qos{ message.get_QoS() };
        types::UInt<TMX_METADATA_PRIORITY_BITS> _priority{ message.get_priority() };
        types::UInt<TMX_METADATA_BASE_BITS> _base{ message.get_base() };
        m.priority(types::pack(_qos, _priority, _base));

        types::UInt<TMX_METADATA_ASSIGNMENT_GROUP_BITS> _grp{ message.get_assignment_group() };
        types::UInt<TMX_METADATA_ASSIGNMENT_ID_BITS> _aid{ message.get_assignment_id() };
        m.group_sequence(types::pack(_grp, _aid));

        m.delivery_count(message.get_attempt());

        // Always assign the TMX message to the tracker
        s.send(m).user_data(new std::shared_ptr<TmxMessage>(*msg));
    }

    delete msg;
}

void TmxQpidProtonClient::on_tracker_settle(proton::tracker &t) {
    TLOG(DEBUG3) << "Enter " << TMX_PRETTY_FUNCTION << " with " << t.container().id();

    auto ptr = (std::shared_ptr<TmxMessage> *)t.user_data();

    // We at least know the topic
    TmxMessage tmp;
    tmp.set_topic(TmxTypeRegistry(t.sender().target().address()).get_namespace().data());

    if (ptr) {
        tmp = *(ptr->get());
        delete ptr;
    }

    std::string msg { "Message from sender "};
    msg.append(t.sender().name());
    msg.append(" ");
    msg.append(enums::enum_name(t.state()).data());
    msg.append(" on topic ");
    msg.append(t.sender().target().address());
    msg.append(" with connection ");
    msg.append(t.sender().container().id());

    this->on_published(this->get_context(t.container().id()), { t.state() == proton::transfer::ACCEPTED ? 0 : (int)t.state(), msg }, tmp);
    t.sender().close();
}

} /* End namespace qpidproton */
} /* End namespace broker */
} /* End namspace tmx */