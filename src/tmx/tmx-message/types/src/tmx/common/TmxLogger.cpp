/*!
 * Copyright (c) 2023 Battelle Memorial Institute
 *
 * All Rights Reserved.
 *
 * @file TmxLogger.cpp
 *
 *  Created on: Jul 03, 2023
 *      @author: Gregpry M. Baumgardner
 */

#include <tmx/common/TmxLogger.hpp>

#include <tmx/common/TmxError.hpp>
#include <tmx/common/TmxFunctor.hpp>
#include <tmx/common/TmxTaskExecutor.hpp>
#include <tmx/common/TmxTypeRegistrar.hpp>
#include <tmx/common/TmxTypeRegistry.hpp>

#include <cstring>
#include <ctime>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <type_traits>

#define TMX_LOGGER_MAX_FILE_NAME 32

namespace tmx {
namespace common {

template <typename _CharT, typename _TimePoint>
std::basic_ostream<_CharT> &_logtime(std::basic_ostream<_CharT> &os, _TimePoint tp)
{
    auto sec = std::chrono::time_point_cast<std::chrono::seconds>(tp);
    if (sec > tp)
        sec = sec - std::chrono::seconds(1);

    struct timeval tv;
    tv.tv_sec = sec.time_since_epoch().count();
    tv.tv_usec = std::chrono::duration_cast<std::chrono::microseconds>(tp - sec).count();

    time_t time = tv.tv_sec;
    struct tm *myTm = localtime(&time);
    char tmBuffer[20];
    strftime(tmBuffer, 20, "%F %T", myTm);

    os << "[" << tmBuffer << "." << std::setfill('0') << std::setw(3) << (tv.tv_usec / 1000) << "] " << std::setfill(' ');

    return os;
}

template <typename _CharT>
std::basic_ostream<_CharT> &_logsource(std::basic_ostream<_CharT> &os, std::string file, std::uint64_t line)
{
    static constexpr int fileMaxLen = TMX_LOGGER_MAX_FILE_NAME;

    if (line > 0)
    {
        file.append(" (");
        file.append(std::to_string(line));
        file.append(")");
    }

    if (file.length() > fileMaxLen)
        os << std::right << std::setw(fileMaxLen) << file.substr(file.size() - fileMaxLen);
    else
        os << std::right << std::setw(fileMaxLen) << file;

    return os;
}

template <typename _CharT>
std::basic_ostream<_CharT> &_loglevel(std::basic_ostream<_CharT> &os, const char *level)
{
    os << " - " << std::left << std::setw(7) << level << ": ";
    return os;
}

static TmxTypeRegistry _logger_reg { "tmx.common.logging" };

std::string _strip_fn(const_string nmspace) {
    auto pIdx = nmspace.find_first_of('(');
    if (pIdx != const_string::npos)
        nmspace = nmspace.substr(0, pIdx);

    auto sIdx = nmspace.find_last_of(' ');
    if (sIdx != const_string::npos)
        nmspace = nmspace.substr(sIdx + 1);

    return { nmspace.data(), nmspace.length() };
}

TmxLogger::TmxLogger(const char *level, const char *nmspace, const char *file, std::uint64_t line) noexcept:
        _time(std::chrono::system_clock::now()), _level(level),
         _nmspace(_strip_fn(nmspace)), _file(file), _line(line) {
    _logtime(this->_stream, this->get_time());
    _logsource(this->_stream, this->get_file().data(), this->get_line());
    _loglevel(this->_stream, this->get_level().data());
}

TmxLogger::TmxLogger(TmxLogLevel level, const char *nmspace, const char *file, std::uint64_t line) noexcept:
         TmxLogger(TmxLogger::to_string(level).data(), nmspace, file, line) { }

std::string const &TmxLogger::get_level() const noexcept {
    return this->_level;
}

std::chrono::system_clock::time_point const &TmxLogger::get_time() const noexcept {
    return this->_time;
}

std::string const &TmxLogger::get_namespace() const noexcept {
    return this->_nmspace;
}

std::string const &TmxLogger::get_file() const noexcept {
    return this->_file;
}

std::uint64_t TmxLogger::get_line() const noexcept {
    return this->_line;
}

std::basic_ostream<char_t> &TmxLogger::stream() noexcept {
    return this->_stream;
}

std::string TmxLogger::str() const noexcept {
    return this->_stream.str();
}

TmxLogLevel TmxLogger::from_string(const char *nm) noexcept {
    auto val = enums::enum_cast<TmxLogLevel>(nm);
    if (val)
        return val.value();
    else
        return TmxLogLevel::OFF;
}

const_string TmxLogger::to_string(TmxLogLevel level) noexcept {
    return enums::enum_name<TmxLogLevel>(level);
}

struct log_level { };
static constexpr auto _log_level = tmx::common::type_short_name<log_level>().data();
static constexpr auto _log_level_seq = tmx::common::enums::enum_sequence<TmxLogLevel>();
static constexpr auto _log_off = tmx::common::enums::enum_name<TmxLogLevel::OFF>().data();

void do_enable(const char *level, TmxTypeRegistry const &reg) {
    (reg / _log_level).register_instance(get_singleton<log_level>(), level);
}

void do_enable(TmxLogLevel level, TmxTypeRegistry const &reg) {
    static auto enums = enums::enum_entries<TmxLogLevel>();
    for (auto l: _log_level_seq.array) {
        do_enable(TmxLogger::to_string(l).data(), reg);

        if (l == level)
            break;
    }
}

void init_namespace(TmxTypeRegistry const &reg) noexcept {
    // Traverse up the tree first. So, if this is the terminal case, just initialize
    if (reg.get_namespace() == _logger_reg.get_namespace()) {
        do_enable(_log_off, reg);
        return;
    }

    auto parent = (reg.get_parent() / _log_level);
    if (!parent.get(_log_off))
        init_namespace(parent.get_parent());

    // Copy all the names from the parent
    for (auto &descr: parent.get_all(typeid(log_level))) {
        if (descr.get_type_namespace() == parent.get_namespace())
            do_enable(descr.get_type_short_name().data(), reg);
    }
}

bool TmxLogger::can_log(const char *level, const char *nmspace) noexcept {
//    auto reg = (_logger_reg / _strip_fn({ nmspace }) / _log_level);
    auto reg = _logger_reg / _log_level;

    // If this namespace has not been initialized, then do so
    if (!reg.get(_log_off))
        init_namespace(reg.get_parent());

    // All correct log levels should now be in place for this namespace
    return reg.get(level);
}

bool TmxLogger::can_log(TmxLogLevel level, const char *nmspace) noexcept {
    return TmxLogger::can_log(TmxLogger::to_string(level).data(), nmspace);
}

void TmxLogger::enable(const char *level, const char *nmspace) noexcept {
    TmxLogger::disable(nmspace);

    auto reg = _logger_reg / _strip_fn({ nmspace });
    if (enums::enum_contains<TmxLogLevel>(level))
        do_enable(TmxLogger::from_string(level), reg);
    else
        do_enable(level, reg);

    TLOG(NOTICE) << (reg.get_parent().get_namespace() == _logger_reg.get_namespace() ? "Root" : nmspace)
                 << " logger enabled at level " << level;
}

void TmxLogger::enable(TmxLogLevel level, const char *nmspace) noexcept {
    TmxLogger::disable(nmspace);

    auto reg = _logger_reg / _strip_fn({ nmspace });
    do_enable(level, reg);

    TLOG(NOTICE) << (reg.get_parent().get_namespace() == _logger_reg.get_namespace() ? "Root" : nmspace)
                 << " logger enabled at level " << TmxLogger::to_string(level);
}

void TmxLogger::disable(const char *nmspace) noexcept {
    TmxTypeRegistry reg = _logger_reg / _strip_fn({ nmspace }) / "log-level";
    for (auto &d: reg.get_all())
        reg.unregister(d.get_type_short_name());
}

void TmxLogger::register_writer(TmxTypeDescriptor const &descriptor) noexcept {
    (_logger_reg / "writers" / descriptor.get_type_short_name()).register_type(descriptor.get_instance(),
                                                                               descriptor.get_typeid(),
                                                                               "|instance|");
}

class TmxFileLogWriter: public TmxLogger::writer {
    typedef TmxFileLogWriter self_type;
public:
    typedef TmxLogger::nmspace_t nmspace_t;
    typedef TmxLogger::level_t level_t;
    typedef TmxLogger::message_t message_t;

    TmxFileLogWriter(): _registry(_logger_reg / "writers" / type_short_name<self_type>().data()) {
        std::shared_ptr<TmxFileLogWriter> _ptr { this, [](auto *) { } };

        TmxLogger::register_writer({ _ptr, typeid(self_type), type_short_name<self_type>().data() });
        (_registry / "log-file").register_instance(_ptr, "-");
    }

    TmxError execute(nmspace_t nmspace, level_t level, message_t msg) const override {
        // Traverse the namespace hierarchy for log file(s) to write to
        TmxTypeRegistry _reg = _registry / nmspace;
        while (_reg.get_namespace().length() >= _registry.get_namespace().length()) {
            bool done = false;
            for (auto &d: (_reg / "log-file").get_all()) {
                if (d.get_type_short_name() == "-")
                    std::cout << msg << std::endl;
                else {
                    std::ofstream os { d.get_type_short_name(), std::ios_base::app };
                    os << msg << std::endl;
                }
                done = true;
            }

            if (done) break;
            _reg = _reg.get_parent();
        }

        return { };
    }

    TmxTypeRegistry _registry;
};

static TmxFileLogWriter _writer;

class TmxLogExector: public TmxTaskExecutor {
    // By default, this is just to be a deferred call
    future<TmxError> exec_async(Functor<TmxError> &&function) noexcept override {
        return std::async(std::launch::deferred, function).share();
    }
};

TmxLogger::~TmxLogger() {
    const TmxLogger *ptr = this;

    // Get an executor for the log
    static TmxLogExector exec;

    // Send this log message to each registered writer
    for (const auto &w: (_logger_reg / "writers").get_all()) {
        if (w.get_type_short_name() == "|instance|")
            common::dispatch(w, std::string(ptr->get_namespace()), std::string(ptr->get_level()), ptr->str());
    }
}

} /* End namespace common */
} /* End namespace tmx */