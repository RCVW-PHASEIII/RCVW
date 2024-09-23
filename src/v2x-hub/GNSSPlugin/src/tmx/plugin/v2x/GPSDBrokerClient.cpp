/*!
 * Copyright (c) 2023 Battelle Memorial Institute
 *
 * All Rights Reserved.
 *
 * @file GPSDBrokerClient.hpp
 *
 *  Created on: Jun 14, 2023
 *      @author: Gregory M. Baumgardner
 *  Edited on: May 7, 2024
 *      @author: Noah Limes
 */

#include <tmx/plugin/v2x/GPSDBrokerClient.hpp>

#include <tmx/common/TmxError.hpp>
#include <tmx/common/TmxTypeDescriptor.hpp>
#include <tmx/common/TmxTypeRegistrar.hpp>
#include <tmx/common/types/Map.hpp>
#include <tmx/message/TmxData.hpp>

#include <tmx/common/TmxFunctor.hpp>
#include <tmx/common/TmxTypeRegistry.hpp>

#include <gps.h>
#include <mutex>
#include <thread>

using namespace tmx::common;
using namespace tmx::common::types;

namespace tmx {
namespace broker {
namespace gpsd {

static TmxTypeRegistrar< GPSDBrokerClient >  _gps_registrar;
static typename types::Properties <TmxBrokerContext *> _ctx_map;

GPSDBrokerClient::GPSDBrokerClient() noexcept {
    this->register_broker("nmea");
    this->register_broker("gpsd");
}

TmxTypeDescriptor GPSDBrokerClient::get_descriptor() const noexcept {
    auto _desc = TmxBrokerClient::get_descriptor();
    return { _desc.get_instance(), typeid(GPSDBrokerClient), "gps" };
}

void GPSDBrokerClient::initialize(TmxBrokerContext &ctx) noexcept {
    if (ctx.get_state() == TmxBrokerState::connected) {
        this->on_destroyed(ctx, {1, "Broker context " + ctx.to_string() + " must first be disconnected"});
        return;
    }

    //extend asio class
    //if scheme is nmea or raw bytes
    //then set message-break param to \n for on_read_message()

    //to message take stuff from GPSDWorker


    _ctx_map[ctx.get_id()] = &ctx;

    // Insert a null object
    ctx[ctx.get_scheme()] = Any();
    ctx.at(ctx.get_scheme()).emplace<struct gps_data_t>().gps_fd = 0;

    // The asynchronous callback will determine the success of the initialization
    std::this_thread::yield();

    // sync across pubs to this context? UNSURE
    ctx.get_receive_sem().wait(ctx.get_receive_lock());
}

void GPSDBrokerClient::on_initialized(tmx::broker::TmxBrokerContext &ctx, const common::TmxError &err) noexcept {
    TmxBrokerClient::on_initialized(ctx, err);

    if (err) {
        //stop connection

        //make sure context forgotten
        _ctx_map.erase(ctx.get_id());

    }

    // This call was synchronized UNSURE
    ctx.get_receive_sem().notify_all();
}

void GPSDBrokerClient::destroy(TmxBrokerContext &ctx) noexcept {
    if (this->is_connected(ctx)) {
        this->disconnect(ctx);
    }

    std::this_thread::yield();

    if (ctx.count(ctx.get_scheme())) {
        ctx.at(ctx.get_scheme()).reset();
        ctx.erase(ctx.get_scheme());
    }

    TmxBrokerClient::destroy(ctx);
}

void GPSDBrokerClient::on_destroyed(tmx::broker::TmxBrokerContext &ctx, const common::TmxError &err) noexcept {

    //make sure context forgotten
    _ctx_map.erase(ctx.get_id());

    TmxBrokerClient::on_destroyed(ctx, err);
}

void GPSDBrokerClient::connect(TmxBrokerContext &ctx, const Any &args) noexcept {
    bool init = ctx.get_state() == TmxBrokerState::initialized;
    if (!init && ctx.get_state() != TmxBrokerState::disconnected)
        return;

    if (ctx.get_state() == TmxBrokerState::connected) {
        this->on_connected(ctx, {1, "Broker context " + ctx.to_string() + " is already connected."});
        return;
    }

    if (!ctx.count(ctx.get_scheme())) {
        this->on_connected(ctx, {1, "Invalid context"});
        return;
    }

    // Create a new gps object
    struct gps_data_t *gps = tmx::common::any_cast<struct gps_data_t>(&(ctx.at(ctx.get_scheme())));
    if (!gps) {
        this->on_connected(ctx, {2, "Context not properly initialized for GPS use."});
        return;
    }

    int ret = gps_open(ctx.get_host().data(), ctx.get_port().empty() ? DEFAULT_GPSD_PORT : ctx.get_port().data(), gps);
    if (ret) {
        std::string err { "Connection to " };
        err.append(ctx.to_string());
        err.append(" failed: ");
        err.append(gps_errstr(ret));
        this->on_connected(ctx, {ret, err});
        return;
    }

    // Start the streaming
    if (!args.has_value()) {
        this->on_connected(ctx, {EINVAL, "No GPSD streaming argument supplied."});
        return;
    }

    message::TmxData v = message::TmxData(args);
    if (!v) {
        this->on_connected(ctx, {EINVAL, "Invalid argument"});
        return;
    }

    ret = gps_stream(gps, v, nullptr);
    if (ret) {
        std::string err { "Stream request to " };
        err.append(ctx.to_string());
        err.append(" failed: ");
        err.append(gps_errstr(ret));
        this->on_connected(ctx, {ret, err});
        return;
    }

    TmxBrokerClient::connect(ctx, args);
}

void GPSDBrokerClient::disconnect(TmxBrokerContext &ctx) noexcept {
    if (!this->is_connected(ctx)) {
        this->on_disconnected(ctx, {EPERM, "Broker context " + ctx.to_string() + " has already been disconnected"});
        return;
    }

    if (!ctx.count(ctx.get_scheme())) {
        this->on_disconnected(ctx, {EPERM, "Invalid context"});
        return;
    }

    struct gps_data_t *gps = tmx::common::any_cast<struct gps_data_t>(&(ctx.at(ctx.get_scheme())));
    if (!gps) {
        this->on_disconnected(ctx, {ENOENT, "Context not properly initialized for GPS use."});
        return;
    }

    if (gps->gps_fd > 0)
        gps_close(gps);

    TmxBrokerClient::disconnect(ctx);
}

void GPSDBrokerClient::publish(TmxBrokerContext &ctx, message::TmxMessage const &msg) noexcept {
    message::TmxMessage *copy = nullptr;
    try {
        copy = new message::TmxMessage(msg);
    } catch (std::exception &ex) {
        this->on_published(ctx, { ex }, msg);
        return;
    }

    if (!this->is_connected(ctx)) {
        std::string err { "No connection established to " };
        err.append(ctx.to_string());
        err.append(". Current state is ");
        err.append(enums::enum_name(ctx.get_state()));
        err.append(".");

        this->on_published(ctx, { EAGAIN, err }, *copy);
        return;
    }

    try {

        //publish somehow
        //im not sure what even NEEDS published to this broker
        auto topic = msg.get_topic().data();
        auto data = msg.get_payload_string();

        // Publish only to GPSD topics
        if (strncmp("GPSD", topic, 4) != 0)
            return;

        // Invoke the handler in a separate thread
        auto invoke_handler_fn = [this, data](std::string _nmspace, TmxBrokerContext *_ctxPtr) {
            if (_ctxPtr) {
                auto ret = dispatch_byname("handle", _nmspace, *_ctxPtr, data);
                if (ret)
                    this->on_error(*_ctxPtr, ret);
            }
        };

        auto nmspace = _gps_registrar.get_registry();
        std::thread _th { invoke_handler_fn, std::string((nmspace / topic).get_namespace().data()), &ctx };
        _th.detach();

        this->on_published(ctx, { }, msg);

    } catch (std::exception &ex) { //if (couldn't publish)
        this->on_published(ctx, { ex }, msg);
    }

}

void GPSDBrokerClient::subscribe(TmxBrokerContext &ctx, common::const_string topic_name, common::TmxTypeDescriptor const &cb) noexcept {
    if (!this->is_connected(ctx)) {
        this->on_subscribed(ctx, EPERM, "Broker context " + ctx.to_string() + " is not connected.", cb);
        return;
    }

    if (!ctx.count(ctx.get_scheme())) {
        this->on_subscribed(ctx, EPERM, "Invalid context", cb);
        return;
    }

    (_gps_registrar.get_registry() / topic_name.data()).register_type(cb.get_instance(), cb.get_typeid(), "handle");

    TmxBrokerClient::subscribe(ctx, topic_name, cb);
}

void GPSDBrokerClient::unsubscribe(TmxBrokerContext &ctx, const_string topic, const TmxTypeDescriptor &cb) noexcept {
    if (!this->is_connected(ctx)) {
        this->on_unsubscribed(ctx, EPERM, "Broker context " + ctx.to_string() + " is not connected.", cb);
        return;
    }

    if (!ctx.count(ctx.get_scheme())) {
        this->on_unsubscribed(ctx, EPERM, "Invalid context", cb);
        return;
    }

(_gps_registrar.get_registry() / topic.data()).unregister("handle");

    TmxBrokerClient::unsubscribe(ctx, topic, cb);
}

static GPSDBrokerClient _gpsd_broker;

} /* End namespace gpsd */
} /* End namespace broker */
} /* End namespace tmx */
