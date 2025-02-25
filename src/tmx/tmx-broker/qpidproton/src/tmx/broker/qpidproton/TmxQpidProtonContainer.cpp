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
#include <tmx/message/codec/serializer/TmxDataSerializer.hpp>

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

#include <atomic>
#include <thread>

using namespace tmx::common;
using namespace tmx::message;
using namespace tmx::message::codec::serializer;

namespace tmx {
namespace broker {
namespace qpidproton {

static std::list<TmxQpidProtonConnection> _containers;
static std::mutex _map_lock;

TmxQpidProtonConnection::TmxQpidProtonConnection(TmxBrokerContext &ctx): _context(ctx),
    _container(new proton::container(*this, ctx.get_id())),
    _thread(std::bind(&TmxQpidProtonConnection::event_loop, this)) {

    // Save this connection back to the context
    static typename types::Properties_::key_t key("connection");
    ctx[key].emplace<std::shared_ptr<TmxQpidProtonConnection> >(this, [](auto *) { });
}

TmxBrokerContext &TmxQpidProtonConnection::context() noexcept {
    return this->_context;
}

proton::container &TmxQpidProtonConnection::container() noexcept {
    return *(this->_container);
}

std::string const &TmxQpidProtonConnection::url() const noexcept {
    return this->_url;
}

TmxBrokerContext &to_context(proton::container const &container) noexcept {
    static TmxBrokerContext _empty;

    std::lock_guard<std::mutex> lock(_map_lock);
    for (auto &c: _containers) {
        if (c.container().id() == container.id())
            return c.context();
    }

    return _empty;
}

void TmxQpidProtonConnection::event_loop() noexcept {
    // Set default options on the container based on the context parameters
    const TmxData params{ this->context().get_parameters() };

    // Construct the URL for first connection
    bool secure = true;

    // Only an explicity unsecured AMQP scheme avoids SSL
    if (std::strcmp("amqp", this->context().get_scheme().c_str()) == 0)
        secure = false;

    this->_url = (secure ? "amqps" : "amqp");
    this->_url += "://" + this->context().get_host();
    if (!this->context().get_port().empty())
        this->_url += ":" + this->context().get_port();

    // The topics and subscriptions may be specified in the URL path
    if (!this->context().get_path().empty()) {
        std::filesystem::path topic;
        std::filesystem::path tmp;

        for (const auto p: std::filesystem::path(this->context().get_path()) ) {
            if (std::strncmp("Subscription", p.native().c_str(), 12) == 0 ||
                    std::strncmp("subscription", p.native().c_str(), 12) == 0) {
                topic = tmp.native();
                tmp.clear();
                continue;
            }

            tmp /= p.filename().native();
        }

        if (!topic.empty() && !tmp.empty())
            TmxData(this->context().get_parameters())["subscription"] = std::string(tmp.native());

        TmxData(this->context().get_parameters())["topics"] =
            std::string(topic.empty() ? tmp.native() : topic.native());

        TLOG(DEBUG2) << topic.native() << "/" << "Subscriptions/" << tmp.native();
    }

    proton::connection_options connOpts;
    connOpts.handler(*this);
    connOpts.container_id(this->context().get_id());

    if (secure) {
        connOpts.sasl_enabled(true);
        connOpts.sasl_allow_insecure_mechs(true);
        connOpts.sasl_allowed_mechs("ANONYMOUS PLAIN");
    }

    if (!this->context().get_user().empty())
        connOpts.user(this->context().get_user());
    if (!this->context().get_secret().empty())
        connOpts.password(this->context().get_secret());


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
    TmxData(this->context().get_parameters())["reconnect-delay"] = delay;
    reconnect.delay(proton::duration(delay));
    if (params["reconnect-delay-multiplier"])
        reconnect.delay_multiplier(params["reconnect-delay-multiplier"].to_float<32>());
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
    sendOpts.delivery_mode(proton::delivery_mode::AT_MOST_ONCE);

    proton::receiver_options recvOpts;
    recvOpts.handler(*this);
    recvOpts.source(srcOpts);
    recvOpts.target(tgtOpts);
    recvOpts.auto_accept(!params["no-auto-accept"]);
    recvOpts.delivery_mode(proton::delivery_mode::AT_MOST_ONCE);
    if (params["credit-window"])
        recvOpts.credit_window(params["credit-window"]);

    container().client_connection_options(connOpts);
    container().sender_options(sendOpts);
    container().receiver_options(recvOpts);

    uint8_t threads = 1;
    if (params["container-threads"])
        threads = params["container-threads"];

    TLOG(DEBUG1) << this->get_broker_info(this->context());

    container().auto_stop(false);

    TmxError ret { EXIT_SUCCESS, "Container for " + container().id() +" has stopped" };
    try {
        TLOG(DEBUG) << "Running the container for " << container().id();
        container().run(threads);
        TLOG(DEBUG) << "Container for " << container().id() << " has stopped.";
    } catch (std::exception &ex) {
        TLOG(ERR) << "Container for " << container().id() << " threw exception: " << ex.what();
        ret = TmxError(ex);
    }

    if (ret)
        TmxBrokerClient::on_error(this->context(), ret);

    std::lock_guard<std::mutex> lock(this->context().get_thread_lock());
    this->on_destroyed(this->context(), ret);
}

void TmxQpidProtonClient::initialize(TmxBrokerContext &ctx) noexcept {
    TmxBrokerState currentState;
    {
        std::lock_guard<std::mutex> lock(ctx.get_thread_lock());
        currentState = ctx.get_state();
    }

    // Only initialize once
    if (currentState == TmxBrokerState::uninitialized) {
        // Add a new container object
        std::lock_guard<std::mutex> lock(_map_lock);
        _containers.emplace_back(ctx);
    }

    // The asynchronous callback will determine the success of the initialization
    // However, in order to ensure the container is running for other operations,
    // this call should be synchronized
    while (currentState == TmxBrokerState::uninitialized) {
        std::this_thread::yield();
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        std::lock_guard<std::mutex> lock(ctx.get_thread_lock());
        currentState = ctx.get_state();
    }
}

void TmxQpidProtonClient::destroy(TmxBrokerContext &ctx) noexcept {
    if (this->is_connected(ctx))
        this->disconnect(ctx);

    TmxBrokerState currentState;
    {
        std::lock_guard<std::mutex> lock(ctx.get_thread_lock());
        currentState = ctx.get_state();
    }

    std::lock_guard<std::mutex> lock(_map_lock);
    _containers.remove_if([id = ctx.get_id()](auto &other) -> bool {
        if (id == other.container().id()) {
           other.container().stop();
           return true;
        }

        return false;
    });

    // The asynchronous callback will determine the success of the destruction
    // However, in order to ensure the container stops properly, this call
    // should be synchronized
    while (currentState != TmxBrokerState::uninitialized) {
        std::this_thread::yield();
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        std::lock_guard<std::mutex> lock(ctx.get_thread_lock());
        currentState = ctx.get_state();
    }
}

void TmxQpidProtonConnection::on_container_start(proton::container &container) {
    TLOG(DEBUG3) << "Enter " << TMX_PRETTY_FUNCTION << " for " << container.id();

    std::lock_guard<std::mutex> lock(this->context().get_thread_lock());
    this->on_initialized(this->context(),
        { EXIT_SUCCESS, "Container for " + container.id() + " has started successfully." });
}

void TmxQpidProtonConnection::on_container_stop(proton::container &container) {
    TLOG(DEBUG3) << "Enter " << TMX_PRETTY_FUNCTION << " for " << container.id();
}

} /* End namespace qpidproton */
} /* End namespace broker */
} /* End namespace tmx */
