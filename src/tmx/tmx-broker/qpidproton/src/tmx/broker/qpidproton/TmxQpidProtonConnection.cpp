/*!
 * Copyright (c) 2024 Battelle Memorial Institute
 *
 * All Rights Reserved.
 *
 * @file TmxQpidProtonConnection.cpp
 *
 *  Created on: Apr 25, 2024
 *      @author: Gregory M. Baumgardner
 */

#include <tmx/broker/qpidproton/TmxQpidProtonClient.hpp>

#include <tmx/common/TmxLogger.hpp>

#include <proton/connection.hpp>
#include <proton/container.hpp>
#include <proton/connection_options.hpp>
#include <proton/duration.hpp>
#include <proton/transport.hpp>
#include <proton/work_queue.hpp>

#include <atomic>
#include <functional>
#include <thread>
#include <mutex>

using namespace tmx::common;
using namespace tmx::message;

namespace tmx {
namespace broker {
namespace qpidproton {

static std::atomic<bool> connecting { false };

proton::connection &TmxQpidProtonClient::get_connection(TmxBrokerContext &ctx, proton::connection *c) noexcept {
    static typename types::Properties_::key_t key { "connection" };
    static proton::connection _no_connection;

    std::lock_guard<std::mutex> lock { ctx.get_thread_lock() };
    if (!ctx.count(key))
        ctx[key].emplace<std::reference_wrapper<proton::connection> >(_no_connection);

    auto val = common::any_cast<std::reference_wrapper<proton::connection> >(&(ctx.at(key)));
    if (!val)
        return _no_connection;

    if (c) val->get() = *c;
    return val->get();
}

void TmxQpidProtonClient::connect(TmxBrokerContext &ctx, types::Any const &params) noexcept {
    auto &container = this->get_container(ctx);
    container.schedule(proton::duration(), [this, &container, &ctx]() {
        // Only try to connect if an existing attempt is not already in progress
        if (!connecting.exchange(true)) {
            if (ctx.get_state() != TmxBrokerState::initialized && ctx.get_state() != TmxBrokerState::disconnected)
                return;

            std::string url = ctx.get_scheme() + "://" + ctx.get_host();
            if (!ctx.get_port().empty())
                url += ":" + ctx.get_port();
            url += ctx.get_path();

            container.connect(url, container.client_connection_options());
        }
    });

    // The asynchronous callback will determine the success of the connection
    std::this_thread::yield();
}

void TmxQpidProtonClient::on_connected(tmx::broker::TmxBrokerContext &ctx, const common::TmxError &err) noexcept {
    // Check the error first
    if (err) {
        TmxBrokerClient::on_connected(ctx, err);
    } else {
        // Check the connection
        auto &connection = this->get_connection(ctx);
        if (connection.active()) {
            std::string msg { "Successfully " };
            if (connection.reconnected())
                msg.append("re");
            msg.append("connected.");

            TmxBrokerClient::on_connected(ctx, { EXIT_SUCCESS, msg });
        } else {
            std::string msg { "Broker context " };
            msg.append(ctx.get_id());
            msg.append(" was not connected properly.");

            TmxBrokerClient::on_connected(ctx, { ECONNABORTED, msg });
        }
    }
}

void TmxQpidProtonClient::on_connection_open(proton::connection &c) {
    TLOG(DEBUG3) << "Enter " << TMX_PRETTY_FUNCTION << " for " << c.container().id();

    // Connection was successful. Save the new object
    this->get_connection(this->get_context(c.container()), &c);
    this->on_connected(this->get_context(c.container()), this->to_error(c.error()));
}

void TmxQpidProtonClient::on_connection_close(proton::connection &c) {
    TLOG(DEBUG3) << "Enter " << TMX_PRETTY_FUNCTION << " for " << c.container().id();

    if (c.error())
        this->on_error(c.error());
}

void TmxQpidProtonClient::on_connection_error(proton::connection &c) {
    TLOG(DEBUG3) << "Enter " << TMX_PRETTY_FUNCTION << " for " << c.container().id();

    this->on_connected(this->get_context(c.container()), this->to_error(c.error()));
}

void TmxQpidProtonClient::disconnect(TmxBrokerContext &ctx) noexcept {
    auto &container = this->get_container(ctx);
    auto &connection = this->get_connection(ctx);
    container.schedule(proton::duration(), [&connection]() {
        if (connection.active()) {
            std::string msg{ "Connection to " };
            msg.append(connection.container_id());
            msg.append(" was manually disconnected.");

            // This is an exceptable disconnection
            TmxError err{ 0, msg };
            connection.close(TmxQpidProtonClient::to_error(err));
        }
    });

    // The asynchronous callback will determine the success of the initialization
    std::this_thread::yield();
}

void TmxQpidProtonClient::on_disconnected(TmxBrokerContext &ctx, const common::TmxError &err) noexcept {
    TmxBrokerClient::on_disconnected(ctx, err);
}

void TmxQpidProtonClient::on_transport_open(proton::transport &t) {
    TLOG(DEBUG3) << "Enter " << TMX_PRETTY_FUNCTION << " for " << t.connection().container().id();
}

void TmxQpidProtonClient::on_transport_close(proton::transport &t) {
    TLOG(DEBUG3) << "Enter " << TMX_PRETTY_FUNCTION << " for " << t.connection().container().id();

    // Per the Qpid Proton docs:
    // The final event for a connection: there will be no more
    // reconnect attempts and no more event functions.
    // Therefore, this must be the final use for the connection
    auto &ctx = this->get_context(t.connection().container());
    std::lock_guard<std::mutex> lock{ ctx.get_thread_lock() };

    ctx.erase(typename types::Properties_::key_t("connection"));
    this->on_disconnected(ctx, this->to_error(t.error()));
}

void TmxQpidProtonClient::on_transport_error(proton::transport &t) {
    TLOG(DEBUG3) << "Enter " << TMX_PRETTY_FUNCTION << " for " << t.connection().container().id();

    auto &ctx = this->get_context(t.connection().container());
    const TmxData params { ctx.get_parameters() };

    // Unless specifically told to reconnect automatically, just close the connection and try to open a new one
    if (params["autoreconnect"])
        this->on_disconnected(this->get_context(t.connection().container()), this->to_error(t.error()));
    else
        t.connection().close();
}

} /* End namespace qpidproton */
} /* End namespace broker */
} /* End namespace tmx */