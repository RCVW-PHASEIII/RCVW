/*!
 * Copyright (c) 2023 Battelle Memorial Institute
 *
 * All Rights Reserved.
 *
 * @file TmxPluginStatus.cpp
 *
 *  Created on: Nov 13, 2023
 *      @author: Gregory M. Baumgardner
 */

#include <tmx/plugin/TmxPlugin.hpp>

#include <tmx/common/TmxLogger.hpp>
#include <tmx/message/TmxData.hpp>
#include <tmx/message/codec/serializer/TmxDataSerializer.hpp>
#include <tmx/plugin/TmxPluginDataUpdate.hpp>

#include <sstream>

using namespace tmx::common;
using namespace tmx::message;
using namespace tmx::message::codec::serializer;

namespace tmx {
namespace plugin {

void TmxPlugin::set_status(const_string key, const types::Any &value, std::mutex *lock) {
    TLOG(DEBUG3) << "Enter " << TMX_PRETTY_FUNCTION << " for " << key << " with " << value;

    if (lock) lock->lock();
    // Save old value
    message::TmxData old { const_cast<types::Any const &>(this->_status[key].get_container()) };

    // Assign new value
    this->_status[key] = value;
    if (lock) lock->unlock();

    // When we set the status programmatically, we should call the registered handlers
    // directly instead of through a channel as there may be no channels at all, or may
    // cause trouble publishing to a topic that is being listened to on the same thread.
    // Therefore, use a TmxPluginDataUpdate to force internal routing only.
    this->broadcast<TmxPluginDataUpdate>({key, old.get_container(), value}, this->get_topic("status"), __FUNCTION__);
    TLOG(DEBUG3) << "Exit " << TMX_PRETTY_FUNCTION;
}

void TmxPlugin::set_status(common::const_string key, const char *str, std::mutex *mutex) {
    this->set_status(key, std::string(str), mutex);
}

message::TmxData TmxPlugin::get_status(const_string key, std::mutex *lock) const {
    TLOG(DEBUG3) << "Enter " << TMX_PRETTY_FUNCTION;

    message::TmxData _ret;

    if (lock) lock->lock();
    const TmxData _ro { this->_status };
    if (!_ro[key].is_empty())
        _ret = _ro[key].get_container();
    if (lock) lock->unlock();

    return std::move(_ret);
}

struct on_status_update { };

template <>
void TmxPlugin::on_message_received<TmxPluginDataUpdate const, on_status_update>(TmxPluginDataUpdate const &upd,
                                                                                 message::TmxMessage const &) {
    TLOG(DEBUG3) << TMX_PRETTY_FUNCTION << " invoked with " << upd.get_container();

    // Skip if this message is poorly formed
    if (upd.get_key().empty()) {
        std::stringstream ss;
        ss << "Incoming status update does not have a key: " << upd;

        this->broadcast<common::TmxError>({ 1, ss.str() }, this->get_topic("error"), "on_status_update");
        return;
    }

    // Need to send updates to the status topic, but only the current value
    // The source should always be the plugin host name
    TmxData msg;
    msg[upd.get_key()] = upd.get_value();
    this->broadcast(msg.get_container(), this->get_topic("status"), "on_status_update", "json");
}

void initialize_status_handlers(TmxPlugin *plugin) {
    if (!plugin) return;

    plugin->register_handler<on_status_update>(plugin->get_topic("status"), plugin,
                                               &TmxPlugin::on_message_received<TmxPluginDataUpdate const, on_status_update>);
}

} /* End namespace plugin */
} /* End namespace tmx */