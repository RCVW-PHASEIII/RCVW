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

#include <proton/container.hpp>
#include <proton/connection_options.hpp>
#include <proton/message.hpp>
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

class TmxQpidProtonSender: public TmxQpidProtonClient, public proton::messaging_handler {
public:
    TmxQpidProtonSender(TmxBrokerContext &ctx, TmxMessage msg): context(ctx), message(msg) { }

    void on_sendable(proton::sender &) override;
    void on_sender_open(proton::sender &) override;
    void on_sender_close(proton::sender &) override;
    void on_sender_detach(proton::sender &) override;
    void on_sender_error(proton::sender &) override;
    void on_tracker_settle(proton::tracker &) override;

    TmxBrokerContext &context;
    TmxMessage message;
    proton::work_queue *_queue = nullptr;
};

static std::list<TmxQpidProtonSender> _senders;
static std::mutex _sender_lock;

void TmxQpidProtonClient::publish(TmxBrokerContext &ctx, TmxMessage const &msg) noexcept {
    std::shared_ptr<TmxQpidProtonConnection> conn;

    if (ctx.count("connection"))
        conn = std::const_pointer_cast<TmxQpidProtonConnection>(types::as<TmxQpidProtonConnection>(ctx.at("connection")));

    if (!conn) {
        this->on_connected(ctx, { EINVAL, "Context " + ctx.get_id() + " was not initialized properly"});
        return;
    }

    // AMPQ topic names have a dot separator instead of slash
    std::string topic { msg.get_topic().c_str() };
    std::replace(topic.begin(), topic.end(), std::filesystem::path::preferred_separator, '.');

    std::lock_guard<std::mutex> lock(_sender_lock);
    _senders.emplace_back(ctx, msg);

    proton::sender_options sendOpts = conn->container().sender_options();
    sendOpts.handler(_senders.back());

    // Handle quality of service
    if (msg.get_QoS()) {
        // TODO: Other settings?

        // Ensure the tracker waits until accepted
        sendOpts.delivery_mode(proton::delivery_mode::AT_LEAST_ONCE);
    }

    // Create a new sender for this topic and attach the message
    conn->container().open_sender(conn->url() + "/" + topic,
        sendOpts, conn->container().client_connection_options());

    // Results will be determined asynchronously
    std::this_thread::yield();
}

void TmxQpidProtonSender::on_sender_open(proton::sender &sender) {
    TLOG(DEBUG3) << "Enter " << TMX_PRETTY_FUNCTION << " for " << sender.container().id();

    // Remember this sender to clean up later
    this->_queue = &(sender.work_queue());
    sender.user_data(&this->message);
}

void TmxQpidProtonSender::on_sender_close(proton::sender &sender) {
    TLOG(DEBUG3) << "Enter " << TMX_PRETTY_FUNCTION << " for " << sender.container().id();

    auto queue = &(sender.work_queue());

    // Remove the sender from the list
    std::lock_guard<std::mutex> lock(_sender_lock);
    _senders.remove_if([queue](auto const &other) ->bool {
       return queue != nullptr && queue == other._queue;
    });

    sender.connection().close();
}

void TmxQpidProtonSender::on_sender_detach(proton::sender &sender) {
    TLOG(DEBUG3) << "Enter " << TMX_PRETTY_FUNCTION << " for " << sender.container().id();
}

void TmxQpidProtonSender::on_sender_error(proton::sender &sender) {
    TLOG(DEBUG3) << "Enter " << TMX_PRETTY_FUNCTION << " for " << sender.container().id();

    this->on_published(this->context, to_error(sender.error()), this->message);
}

void TmxQpidProtonSender::on_sendable(proton::sender &s) {
    TLOG(DEBUG3) << "Enter " << TMX_PRETTY_FUNCTION << " for " << s.container().id();

    // See if there is a message to send
    const auto msg = static_cast<TmxMessage *>(s.user_data());
    s.user_data(nullptr);

    if (msg) {
        if (!s.credit()) {
            std::string err{ "Dropping message with no credits for sender " };
            err.append(s.name());
            err.append(" on topic ");
            err.append(message.get_topic());
            err.append(" with connection ");
            err.append(s.container().id());

            this->on_published(this->context, { EBUSY, err }, message);
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

        proton::timestamp ts {
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
        auto tracker = s.send(m);

        // If no quality of service, just sending is enough
        if (!message.get_QoS())
            this->on_tracker_settle(tracker);
    }
}

void TmxQpidProtonSender::on_tracker_settle(proton::tracker &tracker) {
    TLOG(DEBUG3) << "Enter " << TMX_PRETTY_FUNCTION << " with " << tracker.container().id();

    std::string msg { "Message from sender "};
    msg.append(tracker.sender().name());
    msg.append(" ");
    msg.append(enums::enum_name(tracker.state()).data());
    msg.append(" on topic ");
    msg.append(tracker.sender().target().address());
    msg.append(" with connection ");
    msg.append(tracker.sender().container().id());

    this->on_published(this->context,
        { tracker.state() == proton::transfer::ACCEPTED ? 0 : (int)tracker.state(), msg }, this->message);
    tracker.sender().close();
}

} /* End namespace qpidproton */
} /* End namespace broker */
} /* End namspace tmx */