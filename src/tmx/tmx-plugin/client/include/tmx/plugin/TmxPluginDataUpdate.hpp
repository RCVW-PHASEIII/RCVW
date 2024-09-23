/*!
 * Copyright (c) 2023 Battelle Memorial Institute
 *
 * All Rights Reserved.
 *
 * @file TmxPluginDataUpdate.cpp
 *
 *  Created on: Nov 13, 2023
 *      @author: Gregory M. Baumgardner
 */
#ifndef CLIENT_INCLUDE_TMX_PLUGIN_TMXPLUGINDATAUPDATE_HPP_
#define CLIENT_INCLUDE_TMX_PLUGIN_TMXPLUGINDATAUPDATE_HPP_

#include <tmx/common/TmxLogger.hpp>
#include <tmx/common/types/Any.hpp>
#include <tmx/message/TmxData.hpp>
#include <tmx/message/codec/TmxCodec.hpp>
#include <tmx/plugin/TmxPlugin.hpp>

namespace tmx {
namespace plugin {

/*!
 * @brief A simple DAO for handling plugin configuration or status updates
 *
 * Note that this object, once constructed, is intended to be read-only
 */
class TmxPluginDataUpdate {
public:
    TmxPluginDataUpdate() = delete;
    inline TmxPluginDataUpdate(common::const_string key,
                               common::types::Any const &oldVal, common::types::Any const &newVal) noexcept {
        _data["key"] = std::string(key);
        _data["old"] = oldVal;
        _data["new"] = newVal;
    }
    inline TmxPluginDataUpdate(common::types::Any const &data) noexcept: _data(data) { }
    inline TmxPluginDataUpdate(TmxPluginDataUpdate const &copy) noexcept: _data(copy._data) { }

    inline auto const &get_container() const { return this->_data.get_container(); }
    inline auto get_key() const { return this->_data["key"].to_string(); }
    inline auto get_old_value() const { return this->_data["old"]; }
    inline auto get_new_value() const { return this->_data["new"]; }
    inline auto get_value() const { return this->get_new_value(); }
private:
    message::TmxData _data;
};


/*!
 * @brief Specialization for handling a data update in the plugin
 *
 * These messages are intended for internal use only, thus the message
 * is never actually broadcast to any external channels, but directly
 * invokes the registered handlers for the topic.
 */
template <>
inline void TmxPlugin::broadcast<TmxPluginDataUpdate>(TmxPluginDataUpdate const &upd, common::const_string topic,
                                                      common::const_string source, common::const_string encoding) {
    TLOG(DEBUG3) << "Enter " << TMX_PRETTY_FUNCTION << " with " << message::TmxData(upd.get_container()).to_string();

    message::codec::TmxCodec _codec;
    auto ret = _codec.encode(upd.get_container(), encoding);
    if (ret) {
        this->broadcast<common::TmxError>(ret, this->get_topic("error"), "broadcast");
        return;
    }

    _codec.get_message().set_topic(topic.data());
    _codec.get_message().set_source(source.data());
    return this->invoke_handlers(upd.get_container(), _codec.get_message(), topic);
}

} /* End namespace plugin */
} /* End namespace tmx */

#endif /* CLIENT_INCLUDE_TMX_PLUGIN_TMXPLUGINDATAUPDATE_HPP_ */
