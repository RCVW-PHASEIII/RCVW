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
#include <tmx/common/TmxTypeRegistrar.hpp>
#include <tmx/common/TmxTypeRegistry.hpp>
#include <tmx/message/TmxMessage.hpp>

#include <proton/connection.hpp>
#include <proton/container.hpp>
#include <proton/delivery.hpp>
#include <proton/duration.hpp>
#include <proton/message.hpp>
#include <proton/message_id.hpp>
#include <proton/receiver.hpp>
#include <proton/receiver_options.hpp>
#include <proton/source.hpp>
#include <proton/work_queue.hpp>

#include <thread>

using namespace tmx::common;
using namespace tmx::message;

namespace tmx {
namespace broker {
namespace qpidproton {

static typename types::Properties_::key_t _credit { "credit" };

typedef typename common::TmxFunctor<common::types::Any const &, message::TmxMessage const &>::type::type cb_type;

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

    // Register the handler
    callback_registry(ctx.get_id(), topicName.data()).register_handler(*callback, cb.get_typeid(), cb.get_type_short_name());

    proton::duration wait;
    if (!this->is_connected(ctx))
        wait = TmxData(ctx.get_parameters())["reconnect-delay"];

    auto &container = this->get_container(ctx);
    container.schedule(wait, [this, &ctx, topicName, &cb]() -> void {
        auto &connection = this->get_connection(ctx);
        if (!connection.active()) {
            std::string err{ "No connection established to " };
            err.append(ctx.to_string());
            err.append(". Current state is ");
            err.append(enums::enum_name(ctx.get_state()));
            err.append(".");

            this->on_subscribed(ctx, { ENOTCONN, err }, topicName, cb);
            return;
        }

        // AMPQ topic names have a dot separator instead of slash
        typename types::Properties_::key_t topic { topicName.data() };
        std::replace(topic.begin(), topic.end(), std::filesystem::path::preferred_separator, '.');

        // Address names for Service Bus include subscriptions
        std::string address = topic;
        const TmxData params { ctx.get_parameters() };
        if (params["subscription"])
            address += "/Subscriptions/" + params["subscription"].to_string();

        // See if a receiver is already open
        for (auto r: connection.receivers()) {
            if (r.active() && r.source().address() == address) {
                this->on_subscribed(ctx, { }, topicName, cb);
                return;
            }
        }

        // Open a new receiver
        connection.work_queue().add([&connection, address]() {
            connection.open_receiver(address,
                                     connection.container().receiver_options().name(connection.container().id()));
        });
    });

    // The asynchronous callback will determine the success of the subscription
    std::this_thread::yield();
}

void TmxQpidProtonClient::on_subscribed(TmxBrokerContext &ctx, common::TmxError const &err,
                                        common::const_string topic, common::TmxTypeDescriptor const &cb) noexcept {
    TmxTypeRegistry _tmp { topic.data() };
    TmxBrokerClient::on_subscribed(ctx, err, _tmp.get_namespace().data(), cb);

    // Try again indefinitely if the connection was not ready
    if (err.get_code() == ENOTCONN)
        this->subscribe(ctx, topic, cb);
}

void TmxQpidProtonClient::on_receiver_open(proton::receiver &r) {
    TLOG(DEBUG3) << "Enter " << TMX_PRETTY_FUNCTION << " for " << r.container().id();

    std::string msg { "Successfully opened receiver " };
    msg.append(r.name());
    msg.append(" on topic ");
    msg.append(r.source().address());
    msg.append(" with connection ");
    msg.append(r.container().id());

    this->on_subscribed(this->get_context(r.container()), { 0, msg }, r.source().address(),
                        this->callback_registry(r.container().id(), r.source().address()).get(r.name()));
}

void TmxQpidProtonClient::unsubscribe(TmxBrokerContext &ctx, const_string topic, TmxTypeDescriptor const &cb) noexcept {
    // Close the associated receiver
    for (auto rec: this->get_connection(ctx).receivers()) {
        if (rec.active() && rec.source().address() == topic) {
            rec.drain();

            std::string msg { "Successfully terminated receiver " };
            msg.append(rec.name());
            msg.append(" from topic ");
            msg.append(topic);
            msg.append(" with connection ");
            msg.append(rec.container().id());

            rec.close(this->to_error(TmxError(0, msg)));
        }
    }

    // The asynchronous callback will determine the success of the destruction
    std::this_thread::yield();
}

void TmxQpidProtonClient::on_unsubscribed(TmxBrokerContext &ctx,  common::TmxError const &err,
                                          common::const_string topic,  common::TmxTypeDescriptor const &cb) noexcept {
    // Remove the handler since it no longer will be used
    this->callback_registry(ctx.get_id(), topic.data()).unregister(cb.get_typeid());

    TmxTypeRegistry _tmp { topic.data() };
    TmxBrokerClient::on_unsubscribed(ctx, err, _tmp.get_namespace().data(), cb);
}

void TmxQpidProtonClient::on_receiver_close(proton::receiver &r) {
    TLOG(DEBUG3) << "Enter " << TMX_PRETTY_FUNCTION << " for " << r.container().id();

    this->on_unsubscribed(this->get_context(r.container()), this->to_error(r.error()), r.source().address(),
                          this->callback_registry(r.container().id(), r.source().address()).get(r.name()));
}

void TmxQpidProtonClient::on_receiver_error(proton::receiver &r) {
    TLOG(DEBUG3) << "Enter " << TMX_PRETTY_FUNCTION << " for " << r.container().id();
    r.close(r.error());
}

void TmxQpidProtonClient::on_receiver_detach(proton::receiver &r) {
    TLOG(DEBUG3) << "Enter " << TMX_PRETTY_FUNCTION << " for " << r.container().id();
}

void TmxQpidProtonClient::on_message(proton::delivery &d, proton::message &m) {
    if (!d.settled())
        d.accept();

    types::Any _id;
    _id.emplace<std::string>(d.receiver().container().id());

    // Convert dots back to slashes
    TmxMessage msg;
    msg.set_id(m.content_type());
    msg.set_topic(TmxTypeRegistry(d.receiver().source().address()).get_namespace().data());
    msg.set_source(m.subject());
    msg.set_encoding(m.content_encoding());
    msg.set_timepoint(std::chrono::system_clock::time_point(std::chrono::milliseconds(m.creation_time().milliseconds())));
    msg.set_payload(to_string(m.body()));

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