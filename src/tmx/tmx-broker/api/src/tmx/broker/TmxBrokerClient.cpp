/*!
 * Copyright (c) 2021 Battelle Memorial Institute
 *
 * All Rights Reserved.
 *
 * @file TmxBrokerClient.cpp
 *
 *  Created on: Aug 11, 2021
 *      @author: gmb
 */

#include <tmx/broker/TmxBrokerClient.hpp>

#include <tmx/common/TmxLogger.hpp>
#include <tmx/common/TmxTypeRegistry.hpp>
#include <tmx/message/codec/TmxCodec.hpp>

#include <iostream>

using namespace tmx::common;
using namespace tmx::common::types;

namespace tmx {
namespace broker {

const TmxTypeDescriptor &TmxBrokerClient::empty_descriptor() {
    static std::shared_ptr<const void> _null;
    static TmxTypeDescriptor _singleton { _null, typeid(void), "" };
    return _singleton;
}

static TmxTypeRegistry _clients_reg() {
    return { "tmx.broker.clients" };
}

TmxTypeDescriptor TmxBrokerClient::get_descriptor() const noexcept {
    std::shared_ptr<const void> _ptr { static_cast<const void *>(this), [](auto *) { } };
    return { _ptr, typeid(void), type_fqname(*this).data() };
}

TmxTypeRegistry TmxBrokerClient::callback_registry(const_string id, const_string topic) const noexcept {
    return TmxTypeRegistry(this->get_descriptor().get_type_name()) / id.data() / "callbacks" / topic.data();
}

types::Any TmxBrokerClient::get_broker_info(TmxBrokerContext &ctx) const noexcept {
    message::TmxData info;
    info["broker"] = this->get_descriptor().get_type_name();
    info["state"] = std::string(enums::enum_name(ctx.get_state()));
    info["config"] = ctx.get_defaults();
    info["path"] = ctx.get_path();
    info["port"] = ctx.get_port();
    info["host"] = ctx.get_host();
    info["secret"] = ctx.get_secret();
    info["user"] = ctx.get_user();
    info["scheme"] = ctx.get_scheme();
    info["context"] = ctx.to_string();
    info["id"] = ctx.get_id();

    if (!info["config"])
        info["config"] = types::Null();

    return std::move(info.get_container());
}

void TmxBrokerClient::register_broker(const_string nm) const noexcept {
    auto _descr = this->get_descriptor();

    if (nm.empty())
        nm = _descr.get_type_name();

    _clients_reg().register_type(_descr.get_instance(), _descr.get_typeid(), nm);
}

std::shared_ptr<TmxBrokerClient> TmxBrokerClient::get_broker(TmxBrokerContext const &ctx) noexcept {
    TmxBrokerClient _tmp;
    TmxBrokerContext _copy(ctx);

    if (!ctx) {
        std::string err { "TMX broker context " };
        err.append(ctx.to_string());
        err.append(" is not valid");

        _tmp.on_error(_copy, { 1, err }, false);
        return { };
    }

    auto _descr = _clients_reg().get(ctx.get_scheme());
    if (!_descr) {
        std::string err { "TMX broker client for context " };
        err.append(ctx.to_string());
        err.append(" is not found in namespace ");
        err.append(_clients_reg().get_namespace());

        _tmp.on_error(_copy, { 2, err }, false);
        return { };
    }

    auto _client = _descr.as_instance<TmxBrokerClient>();
    if (!_client) {
        std::string err { "Type " };
        err.append(_descr.get_type_name());
        err.append(" is not an instance of ");
        err.append(type_fqname<TmxBrokerClient>());

        _tmp.on_error(_copy, { 3, err }, false);
        return { };
    }

    return std::const_pointer_cast<TmxBrokerClient>(_client);
}

message::TmxMessage get_message(TmxBrokerContext const &ctx, const_string fn, TmxError const &result) noexcept {
    static TmxTypeRegistry _base { type_namespace(ctx).data() };

    TmxError _tmp { result };
    _tmp[typename TmxError::key_type("context")] = ctx.to_string();

    message::codec::TmxCodec codec;
    codec.encode(static_cast<common::types::Properties<common::types::Any>::value_type const &>(_tmp));
    codec.get_message().set_id(type_fqname(result).data());
    codec.get_message().set_timepoint();
    codec.get_message().set_source(ctx.get_id());
    codec.get_message().set_topic((_base / fn.data()).get_namespace().data());

    return { codec.get_message() };
}

void TmxBrokerClient::on_error(TmxBrokerContext &ctx, TmxError const &result, typename Boolean::value_type fatal) noexcept {
    TLOG(ERR) << ctx.get_id() << ": on_error: (" << result.get_code() << "): " << result.get_message();

//    this->callback(ctx.get_id(), get_message(ctx, __FUNCTION__, result));
    if (result && fatal)
        this->destroy(ctx);
}

void TmxBrokerClient::on_initialized(TmxBrokerContext &ctx, TmxError const &result) noexcept {
    TLOG(DEBUG1) << ctx.get_id() << ": on_initialized: (" << result.get_code() << "): " << result.get_message();

//    this->callback(ctx.get_id(), get_message(ctx, __FUNCTION__, result));
    if (!result)
        ctx.set_state(TmxBrokerState::initialized);
}

void TmxBrokerClient::on_connected(TmxBrokerContext &ctx, TmxError const &result) noexcept {
    TLOG(DEBUG1) << ctx.get_id() << ": " << ctx.to_string() <<
        ": on_connected: (" << result.get_code() << "): " << result.get_message();

//    this->callback(ctx.get_id(), get_message(ctx, __FUNCTION__, result));
    if (!result)
        ctx.set_state(TmxBrokerState::connected);

    // May have to notify waiting semaphores
    ctx.get_receive_sem().notify_all();
    ctx.get_publish_sem().notify_all();
}

void TmxBrokerClient::on_disconnected(TmxBrokerContext &ctx, TmxError const &result) noexcept {
    TLOG(DEBUG1) << ctx.get_id() << ": on_disconnected: (" << result.get_code() << "): " << result.get_message();
//    this->callback(ctx.get_id(), get_message(ctx, __FUNCTION__, result));

    ctx.set_state(TmxBrokerState::disconnected);
}

void TmxBrokerClient::on_subscribed(TmxBrokerContext &ctx, TmxError const &result,
                                    const_string topic, TmxTypeDescriptor const &callback) noexcept {
    TLOG(DEBUG1) << ctx.get_id() << ": " << topic << ": " << callback.get_type_name()
                 << ": on_subscribed: (" << result.get_code() << "): " << result.get_message();

//    TmxError _tmp { result };
//    _tmp[typename TmxError::key_type("topic")] = std::string(topic);
//    _tmp[typename TmxError::key_type("callback")] = callback.get_type_name();
//    this->callback(ctx.get_id(), get_message(ctx, __FUNCTION__, _tmp));
//
    // Must be connected before registered
    if (this->is_connected(ctx) && !result)
        ctx.set_state(TmxBrokerState::registered);
}

void TmxBrokerClient::on_unsubscribed(TmxBrokerContext &ctx, TmxError const &result,
                                      const_string topic, TmxTypeDescriptor const &callback) noexcept {
    TLOG(DEBUG1) << ctx.get_id() << ": " << topic << ": " << callback.get_type_name()
                 << ": on_unsubscribed: (" << result.get_code() << "): " << result.get_message();

//    TmxError _tmp { result };
//    _tmp[typename TmxError::key_type("topic")] = std::string(topic);
//    _tmp[typename TmxError::key_type("callback")] = callback.get_type_name();
//    this->callback(ctx.get_id(), get_message(ctx, __FUNCTION__, _tmp));
}

void TmxBrokerClient::on_published(TmxBrokerContext &ctx, TmxError const &result, message::TmxMessage const &message) noexcept {
    TLOG(DEBUG1) << ctx.get_id() << ": " << message.to_string()
                 << ": on_published: (" << result.get_code() << "): " << result.get_message();

//    TmxError _tmp { result };
//    _tmp[typename TmxError::key_type("tmx-message")] = message.get_container();
//    this->callback(ctx.get_id(), get_message(ctx, __FUNCTION__, _tmp));
}

void TmxBrokerClient::on_destroyed(TmxBrokerContext &ctx, TmxError const &result) noexcept {
    TLOG(DEBUG1) << ctx.get_id() << ": on_destroyed: (" << result.get_code() << "): " << result.get_message();

//    this->callback(ctx.get_id(), get_message(ctx, __FUNCTION__, result));
//    ctx.set_state(TmxBrokerState::uninitialized);
}

} /* End namespace broker */
} /* End namespace tmx */
