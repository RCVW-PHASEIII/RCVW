/*!
 * Copyright (c) 2023 Battelle Memorial Institute
 *
 * All Rights Reserved.
 *
 * @file TmxPluginConfig.cpp
 *
 *  Created on: Nov 13, 2023
 *      @author: Gregory M. Baumgardner
 */

#include <tmx/plugin/TmxPlugin.hpp>

#include <tmx/common/TmxLogger.hpp>
#include <tmx/message/TmxData.hpp>
#include <tmx/message/codec/serializer/TmxDataSerializer.hpp>
#include <tmx/plugin/TmxPluginDataUpdate.hpp>
#include <tmx/plugin/TmxChannel.hpp>

using namespace tmx::common;
using namespace tmx::message;
using namespace tmx::message::codec::serializer;

namespace tmx {
namespace plugin {

void TmxPlugin::set_config(const_string key, types::Any const &value, std::mutex *lock) {
    TLOG(DEBUG3) << "Enter " << TMX_PRETTY_FUNCTION << " for " << key << " with " << value;

    if (lock) lock->lock();
    // Save old value
    message::TmxData old { const_cast<types::Any const &>(this->_config[key].get_container()) };

    // Assign new value
    this->_config[key] = value;
    if (lock) lock->unlock();

    // When we set the config programmatically, we should call the registered handlers
    // directly instead of through a channel as there may be no channels at all, or may
    // cause trouble publishing to a topic that is being listened to on the same thread.
    // Therefore, use a TmxPluginDataUpdate to force internal routing only.
    std::string topic = (TmxTypeRegistry(this->get_topic("config")) / key.data()).get_namespace().data();
    this->broadcast<TmxPluginDataUpdate>({key, old.get_container(), value}, topic, __FUNCTION__);

    TLOG(DEBUG3) << "Exit " << TMX_PRETTY_FUNCTION;
}

void TmxPlugin::set_config(common::const_string key, const char *str, std::mutex *mutex) {
    this->set_config(key, std::string(str), mutex);
}

message::TmxData TmxPlugin::get_config(const_string key, std::mutex *lock) const {
    TLOG(DEBUG3) << "Enter " << TMX_PRETTY_FUNCTION;

    message::TmxData _ret;

    if (lock) lock->lock();
    const TmxData _ro { this->_config };
    if (!_ro[key].is_empty())
        _ret = _ro[key].get_container();
    if (lock) lock->unlock();

    return std::move(_ret);
}

struct on_channel_update { };

template <>
void TmxPlugin::on_message_received<TmxPluginDataUpdate const, on_channel_update>(TmxPluginDataUpdate const &upd,
                                                                                  message::TmxMessage const &) {
    TLOG(DEBUG3) << TMX_PRETTY_FUNCTION << " invoked with " << upd.get_container();

    // TODO: Maybe do a better job of checking active contexts?
    this->_channels.clear();

    // Handle the incoming channel configuration
    const auto &val { upd.get_value() };

    for (auto &v: val.to_array())
        this->_channels.emplace_back(std::make_shared<TmxChannel>(this->get_descriptor(), v));

    TLOG(DEBUG3) << "Exit " << TMX_PRETTY_FUNCTION;
}

struct on_log_level_update { };

template <>
void TmxPlugin::on_message_received<TmxPluginDataUpdate const, on_log_level_update>(TmxPluginDataUpdate const &upd,
                                                                                    message::TmxMessage const &) {
    TLOG(DEBUG3) << TMX_PRETTY_FUNCTION << " invoked with " << upd.get_container();

    // Expect a log level name
    TmxLogger::enable(upd.get_value().to_string());

    TLOG(DEBUG3) << "Exit " << TMX_PRETTY_FUNCTION;
}

struct on_config_update { };

template <>
void TmxPlugin::on_message_received<TmxData const, on_config_update>(TmxData const &data, message::TmxMessage const &) {
    TLOG(DEBUG3) << TMX_PRETTY_FUNCTION << " invoked with " << data;

    for (const auto &v: data.to_map())
        this->set_config(v.first, v.second);

    TLOG(DEBUG3) << "Exit " << TMX_PRETTY_FUNCTION;
}

void initialize_config_handlers(TmxPlugin *plugin) {
    if (!plugin) return;

    plugin->register_handler<on_channel_update>(plugin->get_topic("config/channels"), plugin,
                                &TmxPlugin::on_message_received<TmxPluginDataUpdate const, on_channel_update>);
    plugin->register_handler<on_log_level_update>(plugin->get_topic("config/loglevel"), plugin,
                                &TmxPlugin::on_message_received<TmxPluginDataUpdate const, on_log_level_update>);
    plugin->register_handler<on_config_update>(plugin->get_topic("config"), plugin,
                                &TmxPlugin::on_message_received<TmxData const, on_config_update>);

}

} /* End namespace plugin */
} /* End namespace tmx */