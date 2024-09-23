/*!
 * Copyright (c) 2023 Battelle Memorial Institute
 *
 * All Rights Reserved.
 *
 * @file TmxChannel.cpp
 *
 *  Created on: May 08, 2023
 *      @author: Gregory M. Baumgardner
 */

#include <tmx/plugin/TmxChannel.hpp>

#include <tmx/broker/TmxBrokerClient.hpp>
#include <tmx/broker/TmxBrokerContext.hpp>
#include <tmx/common/TmxFunctor.hpp>
#include <tmx/common/TmxLogger.hpp>
#include <tmx/common/TmxTaskExecutor.hpp>
#include <tmx/common/TmxTypeRegistrar.hpp>
#include <tmx/common/TmxTypeRegistry.hpp>
#include <tmx/common/types/Map.hpp>
#include <tmx/message/TmxData.hpp>
#include <tmx/message/TmxMessage.hpp>
#include <tmx/message/codec/serializer/TmxDataSerializer.hpp>
#include <tmx/plugin/TmxPlugin.hpp>
#include <tmx/plugin/utils/async/TmxRunnable.hpp>
#include <tmx/plugin/utils/async/TmxTaskWorker.hpp>
#include <tmx/plugin/utils/async/TmxWorkerGroup.hpp>

#include <atomic>
#include <boost/asio.hpp>
#include <deque>
#include <memory>
#include <thread>
#include <utility>

#ifndef TMX_MAX_WORKER_THREADS
#define TMX_MAX_WORKER_THREADS 256
#endif

#ifndef TMX_WORKER_DEFAULT_WAIT_MS
#define TMX_WORKER_DEFAULT_WAIT_MS 1024
#endif

using namespace tmx::broker;
using namespace tmx::common;
using namespace tmx::common::types;
using namespace tmx::message::codec::serializer;
using namespace tmx::plugin::utils;

namespace tmx {
namespace plugin {
namespace channels {

// A single registry for channels
struct __channel_registry_type { };
static TmxTypeRegistrar< __channel_registry_type > _channel_registry;

// An empty context to use for errors
static TmxBrokerContext _empty_context;

/*!
 * @brief Retrieve the plugin instance by the registered channel name
 *
 * @param[in] reg The registry for the channel
 * @return The plugin or null if it does not exist
 */
TmxPlugin *get_plugin(TmxTypeRegistry const &reg) {
    auto const &descr = reg.get("plugin");
    auto plugin = std::const_pointer_cast<TmxPlugin>(descr.as_instance<TmxPlugin>());
    if (plugin)
        return plugin.get();

    return nullptr;
}

/*!
 * @brief Retrieve the channel instance by the registered channel name
 *
 * @param[in] reg The registry for the channel
 * @return The channel or null if it does not exist
 */
TmxChannel *get_channel(TmxTypeRegistry const &reg) {
    auto p = get_plugin(reg);
    if (p) {
        std::string id{ std::filesystem::path(reg.get_namespace()).filename().native() };
        auto channel = p->get_channel(id);
        if (channel && channel->get_context())
            return channel.get();
    }

    return nullptr;
}

typedef std::string _channel_id_type;
typedef message::TmxMessage _msg_type;
typedef common::TmxFunctor<_channel_id_type const &, _msg_type const &> fn_type;

class TmxChannelIncomingMessage: public fn_type {
public:
    TmxError execute(_channel_id_type const &ch, _msg_type const &msg) const override {
        auto reg = _channel_registry.get_registry() / ch;
        auto channel = get_channel(reg);
        if (!channel)
            return { EINVAL, "Unable to find channel " + ch + " in registry" };

        channel->connect();

        auto plugin = get_plugin(reg);
        if (!plugin)
            return { EINVAL, "Unable to find plugin for channel " + ch };

        plugin->on_message_received(msg);
        return { };
    }
};

class TmxChannelOutgoingMessage: public fn_type {
public:
    TmxError execute(_channel_id_type const &ch, _msg_type const &msg) const override {
        auto channel = get_channel(_channel_registry.get_registry() / ch);
        if (!channel)
            return { EINVAL, "Unable to find channel " + ch + " in registry" };

        channel->connect();

        auto broker = TmxBrokerClient::get_broker(channel->get_context());
        if (!broker)
            return { EINVAL, "Unable to find messaging broker for channel " + ch };

        broker->publish(channel->get_context(), msg);
        return { };
    }
};

static common::TmxTypeRegistrar<TmxChannelIncomingMessage> _incoming;
static common::TmxTypeRegistrar<TmxChannelOutgoingMessage> _outgoing;

class TmxChannelOnMessageReceived: public TmxFunctor<Any const &, _msg_type const &> {
    TmxError execute(Any const &arg, _msg_type const &msg) const override {
        static const TmxTypeRegistry &_reg = channels::_channel_registry.get_registry();
        const message::TmxData id { arg };
        if (!id)
            return { 1, "No context identifier supplied" };

        // Get the plugin associated with the channel
        auto channel = get_channel(_reg / id.to_string());
        if (channel)
            return channel->execute(_incoming.descriptor(), msg);

        return { 1, "Channel " + id.to_string() + " could not be found" };
    }
};

static common::TmxTypeRegistrar<TmxChannelOnMessageReceived> _msg_receiver;

typedef async::TmxTaskWorker<boost::asio::io_context> worker_t;
typedef async::TmxWorkerGroup<TMX_METADATA_ASSIGNMENT_GROUP_BITS, TMX_METADATA_ASSIGNMENT_ID_BITS> group_t;

static typename types::Properties_::key_t _workers { "workers" };
static typename types::Properties_::key_t _group   { "worker-group" };

} /* End namespace channels */

TmxChannel::TmxChannel(TmxTypeDescriptor const &descriptor, Any const &config) noexcept {
    static const TmxTypeRegistry &_reg = channels::_channel_registry.get_registry();

    TLOG(DEBUG2) << "Incoming config: " << config;

    const message::TmxData cfg { config };
    std::string id;
    if (cfg["id"])
        id = cfg["id"].to_string();

    TmxBrokerContext &ctx = this->_data.emplace<TmxBrokerContext>(cfg["context"].to_string(), id,
                                                                  cfg["config"].get_container());

    auto plugin = std::const_pointer_cast<TmxPlugin>(descriptor.as_instance<TmxPlugin>());
    if (plugin)
        ctx.set_executor(std::shared_ptr<TmxTaskExecutor>(&plugin->get_executor(), [](auto *) { }));

    // Create the workers
    const message::TmxData params { ctx.get_parameters() };
    std::size_t numThreads = params["thread-count"];
    if (numThreads > 0) {
        // Create a worker group
        auto group = ctx[channels::_group].emplace<std::shared_ptr<channels::group_t> >(
                std::make_shared<channels::group_t>());

        auto strategy = enums::enum_cast<async::TmxWorkerAssigmentStrategy>(params["thread-assignment"].to_string());
        if (strategy.has_value())
            group->set_strategy(strategy.value());

        TLOG(DEBUG) << "Launching " << numThreads << " " << enums::enum_name(group->get_strategy())
                    << " worker threads for channel " << ctx.get_id();

        // Due to some unknown race condition, we initialize the workers separately from starting them
        auto &workers = ctx[channels::_workers].emplace<std::vector<channels::worker_t> >();
        for (std::size_t i = 0; i < numThreads && i < TMX_MAX_WORKER_THREADS; i++) {
            std::lock_guard<std::mutex> lock(ctx.get_thread_lock());
            workers.emplace_back(new boost::asio::io_context(1));
        }

        for (std::size_t i = 0; i < workers.size(); i++) {
            workers[i].start();

            std::this_thread::yield();
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
    }

    TLOG(DEBUG) << "Channel context " << ctx.get_id() << ": " <<
                    ctx.to_string() << ": " << static_cast< Properties<Any>::value_type & >(ctx);

    // Assign the plugin type to the context ID
    (_reg / ctx.get_id()).register_type(descriptor.get_instance(), descriptor.get_typeid(), "plugin");
}

TmxChannel::TmxChannel(tmx::plugin::TmxChannel &&moved) noexcept: _data(moved._data)  { }

TmxChannel::~TmxChannel() {
    TmxBrokerContext &ctx = this->get_context();

    TLOG(NOTICE) << "Stopping channel " << ctx.get_id();

    // Stop any worker threads
    if (ctx.count(channels::_workers)) {
        auto workers = types::as<std::vector<channels::worker_t> >(ctx.at(channels::_workers));
        if (workers) {
            TLOG(NOTICE) << "Stopping " << workers->size() << " worker threads for channel " << ctx.get_id();

            for (auto &wrk: *workers)
                wrk.stop();
        }
    }

    auto client = TmxBrokerClient::get_broker(ctx);
    if (client) {
        TLOG(NOTICE) << "Destroying the broker context for channel " << ctx.get_id();

        client->destroy(ctx);
    }
}

TmxBrokerContext &TmxChannel::get_context() noexcept {
    auto ctx = tmx::common::types::as<TmxBrokerContext>(this->_data);
    return ctx ? *ctx : channels::_empty_context;
}

TmxBrokerContext const &TmxChannel::get_context() const noexcept {
    auto ctx = tmx::common::types::as<TmxBrokerContext>(this->_data);
    return ctx ? *ctx : channels::_empty_context;
}

void TmxChannel::disconnect() noexcept {
    // Asynchronously disconnect

    TmxBrokerContext &ctx = this->get_context();
    if (ctx) {
        auto client = TmxBrokerClient::get_broker(ctx);
        if (client)
            client->disconnect(ctx);
    }
}

void TmxChannel::connect(common::types::Any const &params) noexcept {
    TmxBrokerContext &ctx = this->get_context();
    if (ctx) {
        auto client = TmxBrokerClient::get_broker(ctx);
        if (client) {
            if (ctx.get_state() == broker::TmxBrokerState::uninitialized)
                client->initialize(ctx);

            if (!client->is_connected(ctx))
                client->connect(ctx, params);
        }
    }
}

void TmxChannel::write_message(message::TmxMessage const &msg) noexcept {
    TmxBrokerContext &ctx = this->get_context();
    if (ctx) {
        // Check to see if this context is read-only
        const message::TmxData parameters { ctx.get_parameters() };
        if (parameters["read-only"].to_bool())
                return;

        auto client = TmxBrokerClient::get_broker(ctx);
        if (client)
             this->execute(channels::_outgoing.descriptor(), msg);
    }
}

void TmxChannel::read_messages(common::const_string topic) noexcept {
    TmxBrokerContext &ctx = this->get_context();
    if (ctx) {
        // Check to see if this context is write-only
        const message::TmxData parameters { ctx.get_parameters() };
        if (parameters["write-only"].to_bool())
            return;

        this->connect();

        auto client = TmxBrokerClient::get_broker(ctx);
        if (client)
            client->subscribe(ctx, topic, channels::_msg_receiver.descriptor());
    }
}

class TmxDeferredWorkExecutor: public TmxTaskExecutor {
    future<TmxError> exec_async(Functor<TmxError> &&function) {
        return std::async(std::launch::deferred, function);
    }
};

static TmxDeferredWorkExecutor _executor;

common::TmxError TmxChannel::execute(common::TmxTypeDescriptor const &functor, channels::_msg_type const &msg) {
    if (!functor)
        return { EINVAL, "Invalid functor " + functor.get_type_name() };

    // Assign to a worker thread, if possible
    auto &ctx = this->get_context();
    auto exec = ctx.get_executor();

    if (ctx.count(channels::_workers)) {
        auto workers = types::as<std::vector<channels::worker_t> >(ctx.at(channels::_workers));
        if (workers && workers->size()) {
            // Default to the first
            auto ptr = &(workers->front());
            if (ctx.count(channels::_group)) {
                auto group = types::as<channels::group_t>(ctx.at(channels::_group));
                if (group)
                    ptr = &(group->assign(workers->begin(), workers->end(),
                                          msg.get_assignment_group(), msg.get_assignment_id()));
            }

            TLOG(DEBUG3) << ctx.get_id() << ": Assigning " << functor.get_type_name()
                         << " execution to worker " << ptr->get_id();

            exec.reset(ptr, [](auto *) { });
        }
    }

    if (exec) {
        auto future = exec->schedule([this, functor, &ctx, copy = message::TmxMessage(msg)]() -> void {
            TLOG(DEBUG3) << ctx.get_id() << ": Running " << functor.get_type_name()
                         << " execution within thread " << std::this_thread::get_id();
            const message::TmxMessage &msg = copy;
            common::dispatch(functor, channels::_channel_id_type(this->get_context().get_id().data()), msg);
        });

        exec->exec_callback(future);
        return { };
    }

    // No asynchronous context to run in, so use current execution
    return common::dispatch(functor, channels::_channel_id_type(this->get_context().get_id().data()), msg);
}

} /* End namespace plugin */
} /* End namespace tmx */
