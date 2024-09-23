/*!
 * Copyright (c) 2021 Battelle Memorial Institute
 *
 * All Rights Reserved.
 *
 * @file TmxPlugin.cpp
 *
 *  Created on: Aug 11, 2021
 *      @author: Rich Surgenor
 */

#include <tmx/plugin/TmxPlugin.hpp>

#include <tmx/common/TmxTypeHandler.hpp>
#include <tmx/common/types/Map.hpp>
#include <tmx/message/TmxData.hpp>
#include <tmx/message/codec/TmxCodec.hpp>
#include <tmx/message/codec/serializer/TmxDataSerializer.hpp>
#include <tmx/plugin/utils/Clock.hpp>
#include <tmx/plugin/utils/System.hpp>

#include <algorithm>
#include <boost/program_options.hpp>
#include <cstdlib>
#include <chrono>
#include <fstream>
#include <regex>
#include <thread>

using namespace tmx::common;
using namespace tmx::common::types;
using namespace tmx::message::codec::serializer;

namespace tmx {
namespace plugin {

TmxTypeDescriptor TmxPlugin::get_descriptor() const noexcept {
    std::shared_ptr<const void> _ptr { static_cast<const void *>(this), [](auto *) {} };
    return { _ptr, typeid(TmxPlugin), "TmxPlugin" };
}

TmxTypeRegistry TmxPlugin::get_registry() const noexcept {
    static TmxTypeRegistry _base { type_namespace(*this).data() };

    // Take the registry from the descriptor
    // If there is no qualified namespace, then use the base
    auto _descr = this->get_descriptor();
    if (_descr.get_type_name() == _descr.get_type_short_name())
        return { _descr.get_type_name() };
    else
        return { _base / _descr.get_type_short_name() };
}

types::Array<types::Any> TmxPlugin::get_config_description() const noexcept {
    return { };
}

typename String_::value_type TmxPlugin::get_topic(const_string nm) const {
    return { (this->get_registry() / nm.data()).get_namespace().data() };
}

common::types::Array<std::shared_ptr<TmxChannel> > const &TmxPlugin::get_channels() const noexcept {
    return this->_channels;
}

common::types::Array<std::shared_ptr<TmxChannel> > &TmxPlugin::get_channels() noexcept {
    return this->_channels;
}

std::shared_ptr<TmxChannel> TmxPlugin::get_channel(common::const_string id) noexcept {
    auto &channels = this->get_channels();
    for (std::size_t i = 0; i < channels.size(); i++) {
        if (channels.at(i) && channels.at(i)->get_context().get_id() == id)
            return { channels[i] };
    }

    return { };
}

void TmxPlugin::broadcast(message::TmxMessage const &msg) {
    TLOG(DEBUG3) << "Enter " << TMX_PRETTY_FUNCTION;

    // Send out each channel
    for (auto channel: this->_channels) {
        if (channel) {
            // Check to see if this channel should auto publish messages
            const message::TmxData params { channel->get_context().get_parameters() };

            bool autoPub = true;
            if (!params["auto-publish"].is_empty())
                autoPub = params["auto-publish"];

            std::string topicRegex { ".*" };
            if (params["topics"])
                topicRegex = params["topics"].to_string();

            if (autoPub && std::regex_search(msg.get_topic().c_str(), std::regex(topicRegex))) {
                TLOG(DEBUG1) << "Broadcasting: " << msg.to_string()
                             << " to channel " << channel->get_context().get_id();
                channel->write_message(msg);
            }
        }
    }

    TLOG(DEBUG3) << "Exit " << TMX_PRETTY_FUNCTION;
}

void TmxPlugin::broadcast(Any const &data, const_string topic, const_string source, const_string encoding) {
    message::codec::TmxCodec codec;
    TmxError ret = codec.encode(data, encoding);
    if (ret) {
        this->broadcast<TmxError>(ret, this->get_topic("error"), "broadcast");
        return;
    }

    // The source is intended to be a namespace identifier for the plugin host and function
    static std::string hostname;
    if (hostname.empty()) {
        // First, try an environment variable
        const char *env = std::getenv("TMX_SOURCE_NAME");
        if (env) {
            hostname.assign(env);
        } else {
#ifdef TMX_UX_POSIX
            char host[HOST_NAME_MAX];

            // Use the hostname
            if (gethostname(host, HOST_NAME_MAX) == 0) {
                hostname.assign(host);
            } else {
#endif
                int ret = 0;
                hostname.assign(utils::System::ExecCommand("hostname", &ret));
                if (ret)
                    hostname.assign("UNKNOWNHOST");
            }
        }
    }

    TmxTypeRegistry src = TmxTypeRegistry(hostname) / this->get_descriptor().get_type_short_name().data();
    if (!source.empty())
        src = src / source.data();

    codec.get_message().set_topic(topic.data());
    codec.get_message().set_source(src.get_namespace().data());
    this->broadcast(codec.get_message());
}

static void async_callback(TmxError const &err, std::reference_wrapper<TmxPlugin> plugin) noexcept {
    if (err)
        plugin.get().broadcast<TmxError>(err, plugin.get().get_topic("error"), "invoke_handlers");
}

void TmxPlugin::invoke_handlers(types::Any const &data, message::TmxMessage const &msg, const_string topic) {
    if (topic.empty())
        topic = msg.get_topic().data();

    // Invoke the handlers for the given topic
    auto const &_reg = this->get_registry();

    TLOG(DEBUG3) << "Searching through " << (_reg / topic.data()).get_namespace();
    for (auto descr: (_reg / topic.data()).get_all()) {
        // May be a sub-topic
        if (descr.get_path().parent_path().native() != this->get_topic(topic))
            continue;

        // TODO Support non-void returns
        common::dispatch<void>(descr, data, msg);
    }
}

void TmxPlugin::on_message_received(message::TmxMessage const &msg) {
    // Decode the message
    Any data;

    message::codec::TmxCodec codec { msg };
    auto ret = codec.decode(data, msg.get_id());
    if (ret) {
        this->broadcast<TmxError>(ret, this->get_topic("error"), __FUNCTION__);
        return;
    }

    return this->invoke_handlers(std::move(data), msg, msg.get_topic().data());
}

TmxError TmxPlugin::process_args(TmxRunnableArgs const &args) {
    TLOG(DEBUG3) << "Enter " << TMX_PRETTY_FUNCTION;

    std::string usage = "Usage: ";
    if (args.size() > 0 || !args[0].empty())
        // Add program name to the usage
        usage += args[0];

    boost::program_options::options_description desc(usage);
    boost::program_options::variables_map opts;

    // TODO: Move to default get_config_description implementation
    desc.add_options()
            ("help,h", "This help screen.")
            ("manifest,m", boost::program_options::value<std::string>(),
             "Full-path to manifest file. If not specified, the plugin searches for one.")
            ("level,l", boost::program_options::value<std::string>(),
             "Log level, i.e. ERROR, WARNING, INFO, DEBUG, DEBUGn where n=1-4.")
            ("output,o", boost::program_options::value<std::string>()->default_value("-"),
             "Log output file.  Use - for standard output.");

    // Add the configuration parameters to the argument list
    auto config = this->get_config_description();
    for (auto &cDescr: config) {
        const message::TmxData cfg { cDescr };
        if (cfg["key"]) {
            std::string descr = cfg["description"] ? cfg["description"].to_string().c_str() : cfg["key"].to_string().c_str();
            auto defVal = cfg["default"];
            if (defVal.is_empty()) {
                // Add an empty configuration parameter
                this->_config[cfg["key"].to_string()] = types::Null();
                desc.add_options()(cfg["key"].to_string(),
                                   boost::program_options::value<std::string>(), descr.c_str());
            } else {
                this->_config[cfg["key"].to_string()] = defVal;

                if (defVal.is_simple()) {
                    // Check for boolean
                    auto tstBool = types::as_bool(defVal.get_container());
                    if (tstBool) {
                        desc.add_options()(cfg["key"].to_string(),
                                           boost::program_options::value<bool>()->default_value(*tstBool),
                                           descr.c_str());
                    } else {
                        auto str = defVal.to_string();

                        // See if it is a number
                        if (str.length() > 0 && std::isdigit(str.front())) {
                            if (str.find_first_of(".") != std::string::npos) {
                                typedef types::Intmax::value_type type;
                                desc.add_options()(cfg["key"].to_string(),
                                                   boost::program_options::value<type>()->default_value((type) defVal),
                                                   descr.c_str());
                            } else {
                                typedef types::Double::value_type type;
                                desc.add_options()(cfg["key"].to_string(),
                                                   boost::program_options::value<type>()->default_value((type) defVal),
                                                   descr.c_str());
                            }
                        } else {
                            desc.add_options()(cfg["key"].to_string(),
                                               boost::program_options::value<std::string>()->default_value(str.c_str()),
                                               descr.c_str());
                        }
                    }
                }
            }
        }
    }

    try {
        store(boost::program_options::command_line_parser(args).options(desc).run(), opts);
        boost::program_options::notify(opts);

        if (opts.count("help")) {
            std::stringstream ss;
            desc.print(ss);

            return {-1, ss.str()};
        }

        if (opts.count("level"))
            TmxLogger::enable(opts["level"].as<std::string>().c_str());

        filesystem::path _manifest;
        if (opts.count("manifest"))
            _manifest = opts["manifest"].as<std::string>();
        else
            _manifest = "manifest.json";

        if (!filesystem::exists(_manifest)) {
            return { -2, "Could not open manifest file " + absolute(_manifest).native() };
        }

        TLOG(DEBUG) << "Reading in config from " << absolute(_manifest).native();
        std::ifstream f;
        f.exceptions(std::ifstream::failbit | std::ifstream::badbit);

        message::TmxMessage msg;

        auto tp = filesystem::last_write_time(_manifest);
        auto sctp = std::chrono::time_point_cast<std::chrono::system_clock::duration>(
                tp - decltype(tp)::clock::now() + std::chrono::system_clock::now());
        auto sz = filesystem::file_size(_manifest);
        char_t buffer[sz];

        f.open(absolute(_manifest).c_str());
        f.read(buffer, sz);
        f.close();

        // Convert the data to a message
        msg.set_topic(this->get_topic("config"));
        msg.set_source(absolute(_manifest).c_str());
        msg.set_timepoint(sctp);
        msg.set_payload(buffer, sz);

        // Check if the file name extension specifies the encoder
        std::string enc;
        if (_manifest.has_extension()) {
            enc = _manifest.extension().string().substr(1);
        }

        if (enc == "hex" || enc == "b16")
            enc = "base-16";
        else if (enc == "b32")
            enc = "base-32";
        else if (enc == "b64")
            enc = "base-64";
        else if (enc == "txt")
            enc = "string";
        msg.set_encoding(enc);

        types::Any cfg;
        message::codec::TmxCodec codec(msg);
        auto ret = codec.decode(cfg);
        if (ret) {
            std::stringstream ss;
            ss << "Bad manifest: '" << msg.get_container() << "': " << ret.get_message();
            return { ret.get_code(), ss.str() };
        }

        // Add some additional parameters
        message::TmxData params { cfg };
        params["manifest"] = "file://" + msg.get_source();
        this->set_status("manifest", params.get_container());
    } catch (std::exception &ex) {
        // Unable to process arguments
        return { ex };
    }

    // TODO: Build a configuration message from the program options

    // Initialize the system and subscribe to the topics
    this->init();

    // Check to see if there are any errors
    const message::TmxData errStatus { this->get_status("error") };
    if (errStatus) {
        TmxError err { -1, "Unknown error" };

        for (auto const &props: errStatus.to_map())
            err[std::string(props.first)] = props.second;

        return err;
    }

    TLOG(DEBUG3) << "Exit " << TMX_PRETTY_FUNCTION;
    return { };
}

} /* End namespace plugin */
} /* End namespace tmx */
