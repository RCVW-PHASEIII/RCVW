/*!
 * Copyright (c) 2021 Battelle Memorial Institute
 *
 * All Rights Reserved.
 *
 * @file TmxClientLifecycle.cpp
 *
 *  Created on: Aug 26, 2021
 *      @author: gmb
 */

#include <tmx/broker/TmxBrokerClient.hpp>
#include <tmx/common/TmxLogger.hpp>
#include <tmx/common/TmxTaskExecutor.hpp>
#include <tmx/common/types/Map.hpp>

#include <memory>

using namespace tmx::common;
using namespace tmx::common::types;

namespace tmx {
namespace broker {

class TmxDefaultBrokerExecutor: public TmxTaskExecutor {
    future<TmxError> exec_async(Functor<TmxError> &&function) {
        return std::async(std::launch::deferred, function).share();
    }
};

static typename Properties<TmxBrokerContext *>::value_type _context_map;

TmxBrokerContext *TmxBrokerClient::get_context(common::const_string id) const noexcept {
    typename decltype(_context_map)::key_type key { id };
    if (_context_map.count(key))
        return _context_map.at(key);

    return nullptr;
}

void TmxBrokerClient::register_context(TmxBrokerContext &ctx) {
    _context_map[ctx.get_id()] = &ctx;
}

void TmxBrokerClient::unregister_context(TmxBrokerContext &ctx) {
    _context_map.erase(ctx.get_id());
}

bool TmxBrokerClient::is_connected(TmxBrokerContext const &ctx) const noexcept {
    return ctx.get_state() >= TmxBrokerState::connected;
}

bool TmxBrokerClient::is_subscribed(TmxBrokerContext const &ctx, const_string topic) const noexcept {
    return callback_registry(ctx.get_id(), topic).get_all().size() > 0;
}

void TmxBrokerClient::initialize(TmxBrokerContext &ctx) noexcept {
    this->register_context(ctx);
    this->on_initialized(ctx, { EXIT_SUCCESS, std::strerror(EXIT_SUCCESS) });
}

void TmxBrokerClient::destroy(TmxBrokerContext &ctx) noexcept {
    ctx.get_receive_sem().notify_all();

    this->unregister_context(ctx);
    this->on_destroyed(ctx, { EXIT_SUCCESS, std::strerror(EXIT_SUCCESS) });
}

void TmxBrokerClient::reconnect(TmxBrokerContext &ctx) noexcept {
    this->disconnect(ctx);
    this->connect(ctx);
}

void TmxBrokerClient::connect(TmxBrokerContext &ctx, Any const &) noexcept {
    this->on_connected(ctx, { EXIT_SUCCESS, std::strerror(EXIT_SUCCESS) });
}

void TmxBrokerClient::disconnect(TmxBrokerContext &ctx) noexcept {
    this->on_disconnected(ctx, { EXIT_SUCCESS, std::strerror(EXIT_SUCCESS) });
}

void TmxBrokerClient::subscribe(TmxBrokerContext &ctx, const_string topic, TmxTypeDescriptor const &callback) noexcept {
    this->on_subscribed(ctx, { EXIT_SUCCESS, std::strerror(EXIT_SUCCESS) }, topic, callback);
}

void TmxBrokerClient::unsubscribe(TmxBrokerContext &ctx, const_string topic, TmxTypeDescriptor const &callback) noexcept {
    this->on_unsubscribed(ctx, { EXIT_SUCCESS, std::strerror(EXIT_SUCCESS) }, topic, callback);
}

void TmxBrokerClient::publish(TmxBrokerContext &ctx, message::TmxMessage const &message) noexcept {
    this->on_published(ctx, { EXIT_SUCCESS, std::strerror(EXIT_SUCCESS) }, message);
}

void TmxBrokerClient::callback(const_string id, const message::TmxMessage &message) noexcept {
    TLOG(DEBUG3) << "Enter " << TMX_PRETTY_FUNCTION << " on context " << id << " with " << message.to_string();

    static TmxBrokerContext _error_context;
    static TmxDefaultBrokerExecutor _defaultExec;
    static typename Properties_::key_t key { "executor" };

    // See if there is a registered executor to use
    auto ctx = this->get_context(id);
    const types::Any _id { std::string(id.data()) };

    // Invoke the callbacks
    TmxTypeRegistry const &_reg = callback_registry(id, message.get_topic());
    for (const auto &cb: _reg.get_all()) {
        // Do not include entries from other namespaces, i.e. topics
        if (cb.get_type_namespace() != _reg.get_namespace())
            continue;

        TLOG(DEBUG2) << this->get_descriptor().get_type_name() << ": Invoking callback " << cb.get_type_name()
                   << " for incoming message on topic " << message.get_topic();

        auto ret = common::dispatch(cb, _id, message);
        if (ret)
            this->on_error(ctx ? *ctx : _error_context, ret, false);
    }
}

} /* End namespace broker */
} /* End namespace tmx */
