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
#include <proton/reconnect_options.hpp>
#include <proton/transport.hpp>
#include <proton/work_queue.hpp>

#include <functional>
#include <thread>
#include <mutex>

using namespace tmx::common;
using namespace tmx::message;

namespace tmx {
namespace broker {
namespace qpidproton {

void TmxQpidProtonClient::connect(TmxBrokerContext &ctx, types::Any const &params) noexcept {
    std::shared_ptr<TmxQpidProtonConnection> conn;

    if (ctx.count("connection"))
        conn = std::const_pointer_cast<TmxQpidProtonConnection>(types::as<TmxQpidProtonConnection>(ctx.at("connection")));

    if (!conn) {
        this->on_connected(ctx, { EINVAL, "Context " + ctx.get_id() + " was not initialized properly"});
        return;
    }

    // This effectively does nothing as separate connections are made for each sender and receiver
}

void TmxQpidProtonClient::disconnect(TmxBrokerContext &ctx) noexcept {
    std::lock_guard<std::mutex> lock(ctx.get_thread_lock());
    if (this->is_connected(ctx))
        this->on_disconnected(ctx, { EXIT_SUCCESS, "Connection to " + ctx.get_id() + " was manually disconnected" });
}

void TmxQpidProtonConnection::on_connection_open(proton::connection &connection) {
    TLOG(DEBUG3) << "Enter " << TMX_PRETTY_FUNCTION << " for " << connection.container().id();

    auto &ctx = to_context(connection.container());
    std::lock_guard<std::mutex> lock(ctx.get_thread_lock());

    // Update the connectivity state if necessary
    if (!this->is_connected(ctx)) {
        // Check the connection
        if (connection.active()) {
            std::string msg { "Successfully " };
            if (connection.reconnected())
                msg.append("re");
            msg.append("connected to ");
            msg.append(connection.url());

            this->on_connected(ctx, { EXIT_SUCCESS, msg });

            // Periodically wake up this connection
            connection.wake();
        } else {
            std::string msg { "Broker context " };
            msg.append(connection.container().id());
            msg.append(" was not connected properly to ");
            msg.append(connection.url());

            this->on_connected(ctx, { ECONNABORTED, msg });
        }
    }
}

void TmxQpidProtonConnection::on_connection_close(proton::connection &connection) {
    TLOG(DEBUG3) << "Enter " << TMX_PRETTY_FUNCTION << " for " << connection.container().id();
}

void TmxQpidProtonConnection::on_connection_error(proton::connection &connection) {
    TLOG(DEBUG3) << "Enter " << TMX_PRETTY_FUNCTION << " for " << connection.container().id();

    connection.close();
}

void TmxQpidProtonConnection::on_connection_wake(proton::connection &connection) {
    TLOG(DEBUG4) << "Enter " << TMX_PRETTY_FUNCTION << " for " << connection.container().id();

    auto &ctx = to_context(connection.container());
    TmxBrokerState currentState;
    {
        std::lock_guard<std::mutex> lock(ctx.get_thread_lock());
        currentState = ctx.get_state();
    }

    // The connection should be closed if the state was manually changed
    if (connection.active() && currentState == TmxBrokerState::disconnected)
        connection.close();

    // Wake up again after some time
    const TmxData params { ctx.get_parameters() };
    int delay = 500;
    if (params["reconnect-delay"])
        delay = params["reconnect-delay"];

    std::this_thread::yield();
    std::this_thread::sleep_for(std::chrono::milliseconds(delay));
    connection.wake();
}

void TmxQpidProtonConnection::on_transport_open(proton::transport &transport) {
    TLOG(DEBUG3) << "Enter " << TMX_PRETTY_FUNCTION << " for " << transport.connection().container().id();
}

void TmxQpidProtonConnection::on_transport_close(proton::transport &transport) {
    TLOG(DEBUG3) << "Enter " << TMX_PRETTY_FUNCTION << " for " << transport.connection().container().id();
}

void TmxQpidProtonConnection::on_transport_error(proton::transport &transport) {
    TLOG(DEBUG3) << "Enter " << TMX_PRETTY_FUNCTION << " for " << transport.connection().container().id();

    transport.connection().close();
}

} /* End namespace qpidproton */
} /* End namespace broker */
} /* End namespace tmx */