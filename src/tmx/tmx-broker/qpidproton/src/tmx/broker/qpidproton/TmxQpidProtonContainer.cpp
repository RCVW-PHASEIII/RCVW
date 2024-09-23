/*!
 * Copyright (c) 2024 Battelle Memorial Institute
 *
 * All Rights Reserved.
 *
 * @file TmxQpidProtonContainer.cpp
 *
 *  Created on: Apr 25, 2024
 *      @author: Gregory M. Baumgardner
 */

#include <tmx/broker/qpidproton/TmxQpidProtonClient.hpp>

#include <tmx/common/TmxLogger.hpp>
#include <tmx/common/TmxTypeRegistrar.hpp>

#include <proton/container.hpp>
#include <proton/connection.hpp>
#include <proton/connection_options.hpp>
#include <proton/duration.hpp>
#include <proton/receiver_options.hpp>
#include <proton/reconnect_options.hpp>
#include <proton/sender_options.hpp>
#include <proton/source_options.hpp>
#include <proton/target_options.hpp>
#include <proton/terminus.hpp>

#include <thread>

using namespace tmx::common;
using namespace tmx::message;

namespace tmx {
namespace broker {
namespace qpidproton {

// Register the broker client
static TmxTypeRegistrar< TmxQpidProtonClient > _qpid_proton_registrar;

static typename types::Properties<TmxBrokerContext *> _ctx_map;

TmxBrokerContext &TmxQpidProtonClient::get_context(proton::container const &c) noexcept {
    static TmxBrokerContext _empty;

    if (_ctx_map.count(c.id())) {
        TmxBrokerContext *container = _ctx_map.at(c.id());
        if (container)
            return *container;
    }

    return _empty;
}

proton::container &TmxQpidProtonClient::get_container(TmxBrokerContext &ctx) noexcept {
    auto instance = _qpid_proton_registrar.instance();
    static typename types::Properties_::key_t key { "container" };

    if (!ctx.count(key)) {
        auto ptr = new proton::container(static_cast<proton::messaging_handler &>(*this), ctx.get_id());
        return *(ctx[key].emplace<std::shared_ptr<proton::container> >(ptr));
    }

    return *(types::as<proton::container>(ctx.at(key)));
}

static typename types::Properties_::key_t _thread { "thread" };

void TmxQpidProtonClient::initialize(TmxBrokerContext &ctx) noexcept {
    std::lock_guard<std::mutex> lock(ctx.get_thread_lock());
    if (ctx.get_state() != TmxBrokerState::uninitialized)
        return;

    // Save the context for later
    _ctx_map[ctx.get_id()] = &ctx;

    // Create a container for this context
    const TmxData params { ctx.get_parameters() };

    proton::connection_options connOpts;
    connOpts.handler(*this);

    if (std::strcmp("amqps", ctx.get_scheme().c_str()) == 0) {
        connOpts.sasl_enabled(true);
        connOpts.sasl_allow_insecure_mechs(true);
        connOpts.sasl_allowed_mechs("ANONYMOUS PLAIN");
    }
    if (!ctx.get_user().empty())
        connOpts.user(ctx.get_user());
    if (!ctx.get_secret().empty())
        connOpts.password(ctx.get_secret());

    if (params["failover-urls"]) {
        std::vector<std::string> v;
        for (std::size_t i = 0; i < params["failover-urls"].to_uint(); i++)
            v.push_back(params["failover-urls"][i].to_string().c_str());
        connOpts.failover_urls(v);
    }
    if (params["timeout"])
        connOpts.idle_timeout(proton::duration(params["timeout"]));
    if (params["max-frame-size"])
        connOpts.max_frame_size(params["max-frame-size"]);
    if (params["max-sessions"])
        connOpts.max_sessions(params["max-sessions"]);

    proton::reconnect_options reconnect;
    int delay = 500;
    if (params["reconnect-delay"])
        delay = params["reconnect-delay"];
    // This may be used later
    TmxData(ctx.get_parameters())["reconnect-delay"] = delay;
    reconnect.delay(proton::duration(delay));
    if (params["reconnect-delay-multiplier"])
        reconnect.delay_multiplier(params["reconnect-delay-multiplier"]);
    if (params["reconnect-max-delay"])
        reconnect.max_delay(proton::duration(params["reconnect-max-delay"]));
    if (params["reconnect-max-attempts"])
        reconnect.max_attempts(params["reconnect-max-attempts"]);
    connOpts.reconnect(reconnect);

    proton::source_options srcOpts;
    if (params["source-timeout"])
        srcOpts.timeout(proton::duration(params["source-timeout"]));
    else if (params["timeout"])
        srcOpts.timeout(proton::duration(params["timeout"]));
    srcOpts.anonymous(params["anonymous"] || params["source-anonymous"]);
    srcOpts.dynamic(params["dynamic"] || params["source-dynamic"]);

    proton::target_options tgtOpts;
    if (params["target-timeout"])
        tgtOpts.timeout(proton::duration(params["target-timeout"]));
    else if (params["timeout"])
        tgtOpts.timeout(proton::duration(params["timeout"]));
    tgtOpts.anonymous(params["anonymous"] || params["target-anonymous"]);
    tgtOpts.dynamic(params["dynamic"] || params["target-dynamic"]);

    proton::sender_options sendOpts;
    sendOpts.handler(*this);
    sendOpts.source(srcOpts);
    sendOpts.target(tgtOpts);
    sendOpts.auto_settle(!params["no-auto-settle"]);
    if (params["sender-delivery"]) {
        auto mode = enums::enum_cast<proton::delivery_mode::modes>(params["receiver-delivery"].to_string());
        if (mode) sendOpts.delivery_mode(mode.value());
    }

    proton::receiver_options recvOpts;
    recvOpts.handler(*this);
    recvOpts.source(srcOpts);
    recvOpts.target(tgtOpts);
    recvOpts.auto_accept(!params["no-auto-accept"]);
    if (params["receiver-delivery"]) {
        auto mode = enums::enum_cast<proton::delivery_mode::modes>(params["receiver-delivery"].to_string());
        if (mode) recvOpts.delivery_mode(mode.value());
    }
    if (params["credit-window"])
        recvOpts.credit_window(params["credit-window"]);

    auto &c = this->get_container(ctx);
    c.client_connection_options(connOpts);
    c.sender_options(sendOpts);
    c.receiver_options(recvOpts);

    uint8_t threads = 1;
    if (params["container-threads"])
        threads = params["container-threads"];

    // Start up the container in a separate thread
    ctx[_thread].emplace<std::shared_ptr<std::thread> >(new std::thread([&c, threads]() {
        try {
            TLOG(DEBUG) << "Running the container for " << c.id();
            c.run(threads);
            TLOG(DEBUG) << "Container for " << c.id() << " has stopped.";
        } catch (proton::error &ex) {
            TLOG(ERR) << "Container for " << c.id() << " threw exception: " << ex.what();
        }
    }));

    // The asynchronous callback will determine the success of the initialization
    std::this_thread::yield();

    // However, in order to ensure the container is running for other operations,
    // this call should be synchronized
    ctx.get_receive_sem().wait(ctx.get_receive_lock());
}

void TmxQpidProtonClient::on_initialized(TmxBrokerContext &ctx, const common::TmxError &err) noexcept {
    // Set the status
    TmxBrokerClient::on_initialized(ctx, err);

    if (err) {
        // Error upon initialization, make sure the container is stopped
        this->get_container(ctx).stop(this->to_error(err));

        // Make sure the context is forgotten
        _ctx_map.erase(ctx.get_id());
    }

    // This call was synchronized
    ctx.get_receive_sem().notify_all();
}

void TmxQpidProtonClient::on_container_start(proton::container &c) {
    TLOG(DEBUG3) << "Enter " << TMX_PRETTY_FUNCTION << " for " << c.id();

    this->on_initialized(this->get_context(c), { 0, "Container for " + c.id() + " has started successfully." });
}

void TmxQpidProtonClient::destroy(TmxBrokerContext &ctx) noexcept {
    if (this->is_connected(ctx))
        this->disconnect(ctx);

    this->get_container(ctx).stop();

    // The asynchronous callback will determine the success of the destruction
    std::this_thread::yield();

    // Wait until the thread ends
    if (ctx.count(_thread)) {
        auto thread = types::as<std::thread>(ctx.at(_thread));
        if (thread && thread->joinable())
            thread->join();
    }
}

void TmxQpidProtonClient::on_destroyed(TmxBrokerContext &ctx, const common::TmxError &err) noexcept {
    // Make sure the context is forgotten
    _ctx_map.erase(ctx.get_id());

    TmxBrokerClient::on_destroyed(ctx, err);
}

void TmxQpidProtonClient::on_container_stop(proton::container &c) {
    TLOG(DEBUG3) << "Enter " << TMX_PRETTY_FUNCTION << " for " << c.id();

    this->on_destroyed(this->get_context(c), { 0, "Container for " + c.id() + " has been shutdown." });
}

} /* End namespace qpidproton */
} /* End namespace broker */
} /* End namespace tmx */