/*!
 * Copyright (c) 2024 Battelle Memorial Institute
 *
 * All Rights Reserved.
 *
 * @file TmxPluginExec.cpp
 *
 *  Created on: Apr 23, 2024
 *      @author: Gregory M. Baumgardner
 */

#include <tmx/plugin/TmxPlugin.hpp>

#include <tmx/common/TmxFunctor.hpp>
#include <tmx/common/TmxLogger.hpp>
#include <tmx/message/TmxMessage.hpp>
#include <tmx/plugin/utils/async/TmxTaskWorker.hpp>
#include <tmx/plugin/utils/Clock.hpp>

#include <boost/asio.hpp>
#include <csignal>
#include <stdexcept>
#include <thread>

#ifndef TMX_PLUGIN_THREAD_POOL_SIZE
#define TMX_PLUGIN_THREAD_POOL_SIZE 2
#endif

using namespace tmx::common;

namespace tmx {
namespace plugin {
namespace exec {

/*!
 * @brief The task executor for a TMX plugin
 */
static utils::async::TmxTaskWorker<boost::asio::thread_pool> _plugin_exec {
    new boost::asio::thread_pool(TMX_PLUGIN_THREAD_POOL_SIZE)
};

enum class signals : std::int16_t {
    Unknown = -1,
#ifdef TMX_UX_POSIX
    Hangup = SIGHUP,
    Quit = SIGQUIT,
    Trap = SIGTRAP,
#endif
    Abort = SIGABRT,
    FloatingPointException = SIGFPE,
    IllegalInstruction = SIGILL,
    Interrupt = SIGINT,
    SegmentationViolation = SIGSEGV,
    Terminate = SIGTERM
};

template <signals _Sig>
using _sig_tag = std::integral_constant<signals, _Sig>;

std::string _sig_topic(int signal) noexcept {
    static TmxTypeRegistry _reg{ type_fqname<signals>().data() };

    auto sig = enums::enum_cast<signals>(signal);
    std::string nm = "SIG" + std::to_string(signal);
    if (sig)
        nm.assign(enums::enum_name(sig.value()).data());

    return { (_reg / nm).get_namespace().data() };
}

void handle_signal(int signal) {
    TLOG(DEBUG3) << "Enter " << TMX_PRETTY_FUNCTION << " on signal " << signal;

    std::ostringstream errMsg;
    errMsg << _sig_topic(signal) << " signal caught in thread " << std::this_thread::get_id();

    const TmxError err{ signal, errMsg.str() };

    // Invoke handlers on every registered plugin since there is no way to
    // determine which specific one it came from
    for (auto plugin: TmxPlugin::get_all_plugins()) {
        if (plugin)
            plugin->broadcast<TmxError>(err, _sig_topic(signal));
    }
}

} /* End namespace exec */

using namespace tmx::plugin::exec;

template <>
void TmxPlugin::on_message_received<TmxError const, _sig_tag<signals::Interrupt> >(TmxError const &err,
                                                                                   message::TmxMessage const &msg) {
    TLOG(DEBUG3) << "Enter " << TMX_PRETTY_FUNCTION;

    // An interrupt should cause the plugin to re-initialize
    TLOG(NOTICE) << this->get_descriptor().get_type_short_name() << " was interrupted by signal " << err.get_code();
    this->init();
}

template <>
void TmxPlugin::on_message_received<TmxError const, _sig_tag<signals::Terminate> >(TmxError const &err,
                                                                                   message::TmxMessage const &msg) {
    TLOG(DEBUG3) << "Enter " << TMX_PRETTY_FUNCTION;

    // Make sure this was a requested stop
    TLOG(NOTICE) << this->get_descriptor().get_type_short_name() << " terminating from signal " << err.get_code();
    this->stop();
}


template <>
void TmxPlugin::on_message_received<TmxError const, _sig_tag<signals::Abort> >(TmxError const &err,
                                                                               message::TmxMessage const &msg) {
    TmxPlugin::on_message_received<TmxError const, _sig_tag<signals::Terminate> >(err, msg);
}

#ifdef TMX_UX_POSIX

template <>
void TmxPlugin::on_message_received<TmxError const, _sig_tag<signals::Hangup> >(TmxError const &err,
                                                                              message::TmxMessage const &msg) {
    TmxPlugin::on_message_received<TmxError const, _sig_tag<signals::Interrupt> >(err, msg);
}

template <>
void TmxPlugin::on_message_received<TmxError const, _sig_tag<signals::Quit> >(TmxError const &err,
                                                                              message::TmxMessage const &msg) {
    TmxPlugin::on_message_received<TmxError const, _sig_tag<signals::Terminate> >(err, msg);
}

#endif

void TmxPlugin::start() {
    static TmxTypeRegistry reg { type_namespace<signals>().data() };

    auto const &descr = this->get_descriptor();

    // Add the signal handlers
    this->register_handler<_sig_tag<signals::Interrupt> >(_sig_topic((int)signals::Interrupt), this,
                                  &TmxPlugin::on_message_received<TmxError const, _sig_tag<signals::Interrupt> >);
    this->register_handler<_sig_tag<signals::Terminate> >(_sig_topic((int)signals::Terminate), this,
                                  &TmxPlugin::on_message_received<TmxError const, _sig_tag<signals::Terminate> >);
    this->register_handler<_sig_tag<signals::Abort> >(_sig_topic((int)signals::Abort), this,
                                  &TmxPlugin::on_message_received<TmxError const, _sig_tag<signals::Abort> >);

#ifdef TMX_UX_POSIX
    this->register_handler<_sig_tag<signals::Hangup> >(_sig_topic((int)signals::Hangup), this,
                                  &TmxPlugin::on_message_received<TmxError const, _sig_tag<signals::Hangup> >);
    this->register_handler<_sig_tag<signals::Quit> >(_sig_topic((int)signals::Quit), this,
                                  &TmxPlugin::on_message_received<TmxError const, _sig_tag<signals::Quit> >);
#endif
    for (auto sig: enums::enum_values<signals>())
        signal((int)sig, &handle_signal);

    utils::async::TmxRunnable::start();
}

void TmxPlugin::stop() {
    this->unregister_handler<TmxError, _sig_tag<signals::Interrupt> >(_sig_topic((int)signals::Interrupt));
    this->unregister_handler<TmxError, _sig_tag<signals::Terminate> >(_sig_topic((int)signals::Terminate));
#ifdef TMX_UX_POSIX
    this->unregister_handler<TmxError, _sig_tag<signals::Hangup> >(_sig_topic((int)signals::Hangup));
    this->unregister_handler<TmxError, _sig_tag<signals::Quit> >(_sig_topic((int)signals::Quit));
#endif

    // Remove the signal handlers
    for (auto sig: enums::enum_values<signals>())
        signal((int)sig, SIG_DFL);

    utils::async::TmxRunnable::stop();

    // Remove all channels
    this->get_channels().clear();

    exec::_plugin_exec.get_context().stop();
}

common::TmxTaskExecutor &TmxPlugin::get_executor() noexcept {
    return exec::_plugin_exec;
}

TmxError TmxPlugin::main() {
    while (this->is_running())
        std::this_thread::sleep_for(std::chrono::milliseconds(10));

    return { };
}

TmxError TmxPlugin::execute(TmxRunnableArgs const &args) {
    std::string startTime = utils::Clock::ToLocalPreciseTimeString(std::chrono::system_clock::now());

    this->set_status("State", "Starting");
    this->start();
    this->set_status("State", "Initializing");
    auto err = exec::_plugin_exec.schedule(&TmxPlugin::process_args, this, args).get();
    if (err) return err;

    this->set_status("State", "Initialized");
    auto future = exec::_plugin_exec.schedule(&TmxPlugin::main, this);
    if (exec::_plugin_exec.exec_running(future, std::chrono::milliseconds(500)))
        this->set_status("State", "Running");

    this->set_status("StartTime", startTime.c_str());

    exec::_plugin_exec.get_context().attach();

    err = future.get();
    this->set_status("State", "Terminated");
    return err;
}

} /* End namespace plugin */
} /* End namespace tmx */