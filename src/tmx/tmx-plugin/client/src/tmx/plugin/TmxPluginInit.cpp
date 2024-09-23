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
#include <tmx/message/codec/TmxCodec.hpp>
#include <tmx/message/codec/serializer/TmxDataSerializer.hpp>
#include <tmx/plugin/TmxChannel.hpp>
#include <tmx/plugin/utils/Clock.hpp>

#include <regex>

using namespace tmx::common;
using namespace tmx::message;
using namespace tmx::message::codec::serializer;

namespace tmx {
namespace plugin {

// Link in status and config handlers
void initialize_status_handlers(TmxPlugin *);
void initialize_config_handlers(TmxPlugin *);

struct on_error { };

template <>
void TmxPlugin::on_message_received<TmxData const, on_error>(TmxData const &error, message::TmxMessage const &) {
    this->set_status("error", error.get_container());

    // Stop the plugin on a fatal error
    if (error["fatal"].to_bool()) {
        TLOG(CRIT) << "Terminating plugin on fatal error: " << error;

        // Stop 2the main thread if running
        this->stop();
    } else {
        TLOG(ERR) << error;
    }
}

static TmxTypeRegistry _plugin_registry { "tmx.plugin.instances" };

common::types::Array<std::shared_ptr<TmxPlugin> > TmxPlugin::get_all_plugins() noexcept {
    common::types::Array<std::shared_ptr<TmxPlugin> > _ret;

    for (auto const &descr: _plugin_registry.get_all()) {
        auto plugin = descr.as_instance<TmxPlugin>();
        if (plugin) _ret.push_back(std::const_pointer_cast<TmxPlugin>(plugin));
    }

    return _ret;
}

void TmxPlugin::init() {
    // This procedure may invoke default plugin handlers
    initialize_config_handlers(this);
    initialize_status_handlers(this);
    this->register_handler<on_error>(this->get_topic("error"),
                                     this, &TmxPlugin::on_message_received<TmxData const, on_error>);

    // Register this plugin
    auto desc = this->get_descriptor();
    if (desc) {
        this->get_registry().register_type(desc.get_instance(), desc.get_typeid(), "|instance|");
        _plugin_registry.register_type(desc.get_instance(), desc.get_typeid(), desc.get_type_short_name());
    }

    // Initialize the system from the manifest
    if (std::strcmp("Initializing", this->get_status("State").to_string().c_str()) == 0) {
        // Must be synchronous because channel initialization must complete
        // before initializing the brokers
        auto const &manifest = this->get_status("manifest");
        TLOG(INFO) << manifest;
        this->invoke_handlers(types::Any(manifest.get_container()), TmxMessage(), this->get_topic("config"));

        // Dump the current configuration
        TLOG(DEBUG) << "Initial config: " << this->_config.to_string();
    }

    // Set up the
    for (auto v: this->get_registry().get_all()) {
        const auto nm = std::filesystem::proximate(v.get_path(), this->get_registry().get_namespace()).parent_path();

        // Some special cases such as channels for outgoing message topics
        if (nm.empty() ||
                nm.parent_path() == this->get_topic("config") ||
                nm.parent_path() == "tmx/plugin/exec/signals" ||
                nm == this->get_topic("status") ||
                nm == this->get_topic("error"))
            continue;

        TLOG(DEBUG) << v.get_type_name() << ": Topic name " << nm;

        // Accept message from specified topics
        for (auto channel: this->_channels) {
            if (channel) {
                // Check to see if this channel should auto subscribe to this topic
                const message::TmxData params{ channel->get_context().get_parameters() };
                bool autoSub = true;
                if (!params["auto-subscribe"].is_empty())
                    autoSub = params["auto-subscribe"];

                std::string topicRegex { ".*" };
                if (params["topics"])
                    topicRegex = params["topics"].to_string();

                if (autoSub && std::regex_search(nm.c_str(), std::regex(topicRegex))) {
                    TLOG(DEBUG2) << "Reading messages from " << nm
                                 << " on channel " << channel->get_context().get_id();
                    channel->read_messages(nm.string());
                }
            }
        }
    }

    // Give some time to initialize the channels
    std::this_thread::sleep_for(std::chrono::milliseconds(250));
}

} /* End namespace plugin */
} /* End namespace tmx */