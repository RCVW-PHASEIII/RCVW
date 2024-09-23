/*!
 * Copyright (c) 2021 Battelle Memorial Institute
 *
 * All Rights Reserved.
 *
 * @file TmxCtl.cpp
 *
 * Command line tool for TMX
 *
 *  Created on: Jul 25, 2023
 *      @author: gmb
 */

// This need set before any of the includes which may use TmxLogger
#define TMX_DEFAULT_LOG_LEVEL "OFF"

#include <tmx/broker/TmxBrokerClient.hpp>
#include <tmx/broker/TmxBrokerContext.hpp>
#include <tmx/common/TmxError.hpp>
#include <tmx/common/TmxLogger.hpp>
#include <tmx/common/TmxTypeRegistry.hpp>
#include <tmx/common/TmxTypeRegistrar.hpp>
#include <tmx/common/types/Any.hpp>
#include <tmx/message/TmxData.hpp>
#include <tmx/message/codec/TmxCodec.hpp>
#include <tmx/message/codec/serializer/TmxDataSerializer.hpp>
#include <tmx/plugin/utils/async/TmxRunnable.hpp>

#include <algorithm>
#include <boost/program_options.hpp>
#include <fstream>
#include <iostream>
#include <pwd.h>
#include <regex>
#include <signal.h>
#include <sstream>
#include <string>
#include <sys/stat.h>
#include <unistd.h>
#include <vector>

using namespace tmx::common;
using namespace tmx::message::codec::serializer;

namespace tmx {
namespace plugin {
namespace tmxctl {

message::codec::TmxCodec codec;
std::atomic<bool> received { false };

class Callback: tmx::common::TmxFunctor<types::Any const &, message::TmxMessage const &> {
public:

    common::TmxError execute(types::Any const &, message::TmxMessage const &incoming) const override {
        TLOG(DEBUG) << "Received: " << incoming;
        codec.get_message() = incoming;
        received = true;
        return { };
    }
};

static TmxTypeRegistrar<Callback> _callback;

class TmxCtl: public utils::async::TmxRunnable {
public:
    TmxCtl() = default;

    TmxError process_args(TmxRunnableArgs const &args) override {
        std::string usage = "Usage";
        if (args.size() > 0 || !args[0].empty())
            // Add program name to the usage
            usage += ": " + args[0];

        boost::program_options::options_description desc(usage);
        boost::program_options::variables_map opts;

        // Get the plugin user password entry
        auto plugin_dir = ".";
        for (struct passwd *entry = getpwent(); entry; entry = getpwent()) {
            if (std::strcmp("plugin", entry->pw_name) == 0 && entry->pw_dir)
                plugin_dir = entry->pw_dir;
        }

        desc.add_options()
                ("help,?", "This help screen.")
                ("level,l", boost::program_options::value<std::string>(),
                        "Log level, i.e. ERROR, WARNING, INFO, DATA, DEBUG, DEBUGn where n=1-4.")
                ("output,o", boost::program_options::value<std::string>()->default_value(TMX_DEFAULT_CODEC),
                        "Encode the output results as necessary using the specified encoder.")
                ("show,s", boost::program_options::value<std::string>(),
                        "Show the TMX registry namespace.")
                ("context,c", boost::program_options::value<std::string>(),
                        "Set the broker context")
                ("info,i", "Show the broker information")
                ("codec,C", "Show the available TMX codec.")
                ("broker,B", "Show the available TMX brokers.")
                ("plugins,P", "Show the TMX plugins that are or have been attached to the broker context.")
                ("list,L", "List the plugin manifest information from the local machine")
                ("start", "Start the specified plugin(s)")
                ("stop", "Stop the specified plugin(s)")
                ("arg,a", boost::program_options::value<std::vector<std::string> >(),
                        "Set a command line argument for the plugin(s)")
                ("encode,e", boost::program_options::value<std::string>(),
                    "Decode the encoded value given on standard input using the specified encoder.")
                ("decode,d", boost::program_options::value<std::string>(),
                        "Decode the encoded value given on standard input using the specified decoder.")
                ("no-tmx", "Do not display the complete TMX message when encoding.")
                ("id,I", boost::program_options::value<std::string>(),
                        "Set the type identifier for the TMX message when encoding or decoding. Defaults to empty string.")
                ("source,S", boost::program_options::value<std::string>(),
                        "Set the source for the TMX message when encoding or decoding. Defaults to empty string.")
                ("topic,T", boost::program_options::value<std::string>(),
                        "Set the topic name for the TMX message when encoding or decoding. Defaults to empty string.")
                ("time,t", boost::program_options::value<std::uint64_t>(),
                        "Set the timestamp for the TMX message when encoding or decoding. Defaults to current time.")
                ("plugin-dir,D", boost::program_options::value<std::string>()->default_value(plugin_dir),
                        "The default plugin directory to look through")
                ("input", boost::program_options::value<std::vector<std::string> >(),
                 "The specific input to operate on. "
                 "Depending on other options, this could be a TMX plugin "
                 "or some message data to encode/decode.")
        ;

        message::TmxData output;
        broker::TmxBrokerContext ctx;

        try {
            boost::program_options::positional_options_description pargs;
            pargs.add("input", -1);

            // Need a sub-array to get rid of the program name
            TmxRunnableArgs copy { args.begin() + 1, args.end() };

            store(boost::program_options::command_line_parser(copy).options(desc).positional(pargs).run(), opts);
            boost::program_options::notify(opts);

            if (opts.count("help")) {
                std::stringstream ss;
                desc.print(ss);
                return {-1, ss.str()};
            }

            if (opts.count("level"))
                common::TmxLogger::enable(opts["level"].as<std::string>().c_str());

            std::vector<std::string> _input;
            if (opts.count("input"))
                _input = opts["input"].as< std::vector<std::string> >();

            if (opts.count("context"))
                ctx = broker::TmxBrokerContext(opts["context"].as<std::string>());
            if (opts.count("id"))
                codec.get_message().set_id(opts["id"].as<std::string>());
            if (opts.count("source"))
                codec.get_message().set_source(opts["source"].as<std::string>());
            if (opts.count("topic"))
                codec.get_message().set_topic(opts["topic"].as<std::string>());
            if (opts.count("time"))
                codec.get_message().set_timestamp(opts["time"].as<std::uint64_t>());
            else
                codec.get_message().set_timepoint();

            if (opts.count("list")) {
                get_plugins(output, opts["plugin-dir"].as<std::string>(), _input, true);
            } else if (opts.count("start")) {
                message::TmxData data;
                get_plugins(data, opts["plugin-dir"].as<std::string>(), _input, false);

                // Get arguments
                std::vector<std::string> _args;
                if (opts.count("arg"))
                    _args = opts["arg"].as<std::vector<std::string> >();

                auto props = tmx::common::any_cast< types::Properties<types::Any> >(&(data.get_container()));
                if (props) {
                    for (auto &p: *props) {
                        if (int ret = chdir(p.first)) {
                            output[p.first]["ReturnCode"] = ret;

                            if (errno)
                                output[p.first]["Error"] = std::string(std::strerror(errno));

                            continue;
                        }

                        const message::TmxData manifest { p.second };
                        if (manifest["exe"]) {
                            std::filesystem::path path = std::filesystem::absolute(
                                    std::filesystem::relative(manifest["exe"].to_string().c_str()));

                            char *_argv[_args.size() + 2];
                            _argv[0] = strdup(path.native().c_str());
                            for (std::size_t i = 1; i <= _args.size(); i++)
                                _argv[i] = strdup(_args[i].c_str());
                            _argv[_args.size() + 1] = nullptr;

                            TLOG(INFO) << "Starting " << path;

                            int ret = execv(_argv[0], _argv);
                            output[p.first]["ReturnCode"] = ret;
                            if (ret && errno)
                                output[p.first]["Error"] = std::string(std::strerror(errno));

                            free(_argv[0]);
                            _argv[0] = nullptr;
                            for (std::size_t i = 1; i <= _args.size(); i++) {
                                free(_argv[i]);
                                _argv[i] = nullptr;
                            }
                        }
                    }
                }
            } else if (opts.count("stop")) {
                message::TmxData data;
                get_plugins(data, opts["plugin-dir"].as<std::string>(), _input, true);

                auto props = tmx::common::any_cast< types::Properties<types::Any> >(&(data.get_container()));
                if (props) {
                    for (auto &p: *props) {
                        const message::TmxData ps { p.second };
                        for (std::size_t i = 0; i < (std::size_t) ps["pid"]; i++) {
                            pid_t pid = ps["pid"][i].to_uint();
                            TLOG(INFO) << "Stopping process " << ps["pid"][i].to_string();
                            if (pid > 0) {
                                int ret = kill(pid, SIGTERM);
                                output[p.first]["ResultCode"] = ret;
                                if (ret && errno)
                                    output[p.first]["Error"] = std::string(std::strerror(errno));
                            }
                        }
                    }
                }
            } else if (opts.count("show")) {
                this->show_registry(output[opts["show"].as<std::string>()].get_container(),
                                    opts["show"].as<std::string>());
            } else if (opts.count("info") || opts.count("plugins")) {
                auto broker = broker::TmxBrokerClient::get_broker(ctx);
                if (!broker) return { 1, "No broker context" };

                // Connect the broker
                broker->initialize(ctx);
                broker->connect(ctx);

                // Wait for the connection to be established
                bool ret = ctx.get_receive_sem().wait_for(ctx.get_receive_lock(), std::chrono::seconds(5),
                                                          [broker, &ctx]() {
                                                              return broker->is_connected(ctx);
                                                          });

                if (ret) {
                    if (opts.count("info")) {
                        output = broker->get_broker_info(ctx);
                    } else {
                        const message::TmxData info{ broker->get_broker_info(ctx) };

                        std::size_t j = 0;
                        for (std::size_t i = 0; i < (std::size_t) (info["topics"]); i++) {
                            TLOG(DEBUG) << "Retrieved " << info["topics"][i];

                            auto nm = info["topics"][i]["name"].to_string();
                            std::cmatch cm;
                            std::regex_match(nm.c_str(), cm, std::regex("^(tmx/.*)/status$"));
                            // TODO: Determine the actual plugin name
                            if (cm.size())
                                output[j++] = cm.str(1);
                        }
                    }
                }

                broker->disconnect(ctx);
                broker->destroy(ctx);

                if (!ret) return { ETIMEDOUT, "Timed out waiting for broker connection" };
            } else if (opts.count("codec")) {
                this->show_registry(output["tmx/message/codec/TmxEncoder"].get_container(), "tmx.message.codec.encoders", true);
                this->show_registry(output["tmx/message/codec/TmxDecoder"].get_container(), "tmx.message.codec.decoders", true);
            } else if (opts.count("broker")) {
                this->show_registry(output["tmx/broker/TmxBrokerClient"].get_container(), "tmx.broker.clients", true);
            } else {
                if (ctx) {
                    typedef TmxFunctor<types::Any const, message::TmxMessage const> cb_type;

                    // Read messages off the broker
                    auto broker = broker::TmxBrokerClient::get_broker(ctx);
                    if (!broker) return { 1, "No broker context" };

                    // Connect the broker
                    broker->initialize(ctx);
                    broker->connect(ctx);

                    std::size_t count = 0;
                    bool ret = ctx.get_receive_sem().wait_for(ctx.get_receive_lock(), std::chrono::seconds(5),
                                                              [broker, &ctx]() {
                                                                  return broker->is_connected(ctx);
                                                              });
                    if (ret) {
                        broker->subscribe(ctx, codec.get_message().get_topic(), _callback.descriptor());
                        while (!received) usleep(1000);
                        broker->unsubscribe(ctx, codec.get_message().get_topic(), _callback.descriptor());
                    }

                    broker->disconnect(ctx);
                    broker->destroy(ctx);

                    if (!ret) return { ETIMEDOUT, "Timed out waiting for broker connection" };
                } else {
                    const auto &str = read_in();
                    codec.get_message().set_payload(str.data(), str.length());
                }

                TLOG(INFO) << "Received message: " << codec.get_message();

                if (opts.count("decode")) {
                    codec.get_message().set_encoding(opts["decode"].as<std::string>());
                    auto ret = codec.decode(output.get_container(), codec.get_message().get_id());
                    if (ret) return ret;
                }

                if (opts.count("encode")) {
                    types::Any data;
                    if (opts.count("decode")) {
                        codec.get_message().set_encoding("");
                        data = output.get_container();
                    } else {
                        data.emplace<types::String8>(codec.get_message().get_payload_string());
                    }

                    auto ret = codec.encode(data, opts["encode"].as<std::string>());
                    if (ret) return ret;
                }

                if (!opts.count("no-tmx")) {
                    output = codec.get_message().get_container();
                } else {
                    // Only write out the encoded payload as a string
                    output.get_container().emplace<types::String8>((codec.get_message().get_payload_string()).data());
                }
            }

            auto encoder = message::codec::TmxEncoder::get_encoder(opts["output"].as<std::string>());
            if (!encoder)
                return { 1, "Missing output encoder: " + opts["output"].as<std::string>() };

            auto ret = encoder->encode(output.get_container(), std::cout);
            if (ret) return ret;

            std::cout << std::endl;
            return { };
         } catch (std::exception &ex) {
            // Unable to process arguments
            return { ex };
        }
    }

    TmxError main() override {
        return { };
    }

private:
    // These are the operations that the TMX controller handles
    types::Any get_level(common::types::Any const &);
    types::Any set_level(common::types::Any const &);

    void show_registry(types::Any &data, std::string nmspace, bool shortName = false) {
        auto &array = data.emplace< types::Array<common::types::Any> >();
        for (const auto &desc: common::TmxTypeRegistry(nmspace).get_all())
            array.push_back(std::string(shortName ? desc.get_type_short_name() : desc.get_type_name()));
    }

    std::string read_in(std::basic_istream<common::char_t> &in = std::cin) {
        std::stringstream out;

        std::size_t lineNum = 0;
        std::string line;
        while (std::getline(in, line)) {
            if (lineNum++)
                out << std::endl;

            out << line;
        }

        return out.str();
    }

    void get_plugins(message::TmxData &data, std::string dir, std::vector<std::string> const &plugins, bool ps) {
        // Iterate the directory
        for (const auto &dir: std::filesystem::directory_iterator(dir)) {
            if (std::filesystem::exists(dir.path() / "manifest.json")) {
                message::TmxData manifest;

                // Read in the manifest
                std::ifstream json { (dir.path() / "manifest.json").native() };
                auto ret = message::codec::TmxDecoder::get_decoder("json")->decode(manifest.get_container(), json);
                if (ret) {
                    ret[std::string("status")] = std::string("Unknown");
                    data[std::filesystem::canonical(dir.path()).native()] = ret;
                } else {
                    // Only add if the name matches
                    if (plugins.size() &&
                        std::find(plugins.begin(), plugins.end(), manifest["name"].to_string()) == plugins.end())
                        continue;

                    if (ps) {
                        // Check for running status
                        std::size_t i = 0;
                        for (const auto &procdir: std::filesystem::directory_iterator("/proc")) {
                            auto path = procdir.path() / "exe";

                            if (access(path.c_str(), R_OK) == 0) {
                                // The symlinks must match
                                auto exe = dir.path() / manifest["exe"].to_string().c_str();
                                if (std::filesystem::exists(exe) && std::filesystem::is_symlink(path)) {
#ifdef _WIN32
                                    // Check that the names match
                                    if (std::strcmp(exe.c_str(), path.c_str()) == 0) {
#else
                                    struct stat linkInfo;
                                    struct stat exeInfo;

                                    // Check the inodes
                                    if (stat(path.c_str(), &linkInfo) == 0 && stat(exe.c_str(), &exeInfo) == 0 &&
                                        linkInfo.st_ino == exeInfo.st_ino) {
#endif
                                        manifest["pid"][i++] = path.parent_path().filename().native();
                                    }
                                }
                            }
                        }

                        manifest["status"] = std::string(i > 0 ? "Running" : "Stopped");
                    }

                    data[std::filesystem::canonical(dir.path()).native()] = manifest;
                }
            }
        }
    }
};

} /* namespace tmxctl */
} /* namespace plugin */
} /* namespace tmx */

int main(int argc, char **argv) {
    tmx::plugin::tmxctl::TmxCtl _ctl;
    return tmx::plugin::run(_ctl, argc, argv);
}