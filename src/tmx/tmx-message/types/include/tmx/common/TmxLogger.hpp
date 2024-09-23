/*!
 * Copyright (c) 2023 Battelle Memorial Institute
 *
 * All Rights Reserved.
 *
 * @file TmxLogger.hpp
 *
 *  Created on: Jul 03, 2023
 *      @author: Gregpry M. Baumgardner
 */

#ifndef LOGGING_INCLUDE_TMX_COMMON_TMXLOGGER_HPP_
#define LOGGING_INCLUDE_TMX_COMMON_TMXLOGGER_HPP_

#include <tmx/platform.hpp>

#include <tmx/common/TmxError.hpp>
#include <tmx/common/TmxFunctor.hpp>
#include <tmx/common/TmxTypeDescriptor.hpp>
#include <tmx/common/types/Any.hpp>

#include <chrono>
#include <sstream>
#include <string>

#ifndef TMX_DEFAULT_LOG_LEVEL
#define TMX_DEFAULT_LOG_LEVEL "INFO"
#endif

namespace tmx {
namespace common {

enum class TmxLogLevel : std::int8_t {
    OFF = std::numeric_limits<std::int8_t>::min(),
    EMERG = 0,
    ALERT,
    CRIT,
    ERR,
    WARN,
    NOTICE,
    INFO,
    DATA,
    DEBUG,
    DEBUG1,
    DEBUG2,
    DEBUG3,
    DEBUG4
};

/*!
 * @brief A class for logging messages in a TMX application
 *
 * This class is supposed to only be instantiated if the level of
 * the message is at or below the effective log level for the
 * namespace. Once destructed, the message will be passed to a
 * set of configured log writers. Therefore, this class should
 * not be created directly, but through the use of the TLOG macro.
 *
 * While the pre-defined logging levels are generally preferred
 * in order to support the hierarchy, this class is actually
 * extensible to custom logging levels specified by name. The
 * logging configuration for that level must still be set.
 */
class TmxLogger {
public:
    typedef std::string nmspace_t;
    typedef std::string level_t;
    typedef std::string message_t;

    /*!
     * @brief The type definition for a log writer
     */
    typedef TmxFunctor<nmspace_t, level_t, message_t> writer;

    /*!
     * @brief Constructor for the logger
     *
     * @param[in] The log level for this log message
     * @param[in] The namespace for this log message
     * @param[in] The source file for this log message
     * @param[in] The line number for this log message
     */
    TmxLogger(const char *, const char *, const char *, std::uint64_t) noexcept;

    /*!
     * @brief Constructor for the logger
     *
     * @param[in] The log level for this log message
     * @param[in] The namespace for this log message
     * @param[in] The source file for this log message
     * @param[in] The line number for this log message
     */
    TmxLogger(TmxLogLevel, const char *, const char *, std::uint64_t) noexcept;

    /*!
     * @brief No copy constructor
     */
    TmxLogger(TmxLogger const &) = delete;

    /*!
     * @brief Destructor invokes the writers
     */
    ~TmxLogger();

    /*!
     * @brief No copy assignment
     */
    TmxLogger &operator=(TmxLogger const &) = delete;

    /*!
     * @return The timestamp for this log message
     */
    std::chrono::system_clock::time_point const &get_time() const noexcept;

    /*!
     * @return The log level for this log message
     */
    std::string const &get_level() const noexcept;

    /*!
     * @return The namespace for this log message
     */
    std::string const &get_namespace() const noexcept;

    /*!
     * @return The source file for this log message
     */
    std::string const &get_file() const noexcept;

    /*!
     * @return The source line number for this log message
     */
    std::uint64_t get_line() const noexcept;

    /*!
     * @return The data stream to write to
     */
    std::basic_ostream<char_t> &stream() noexcept;
    std::string str() const noexcept;

    static TmxLogLevel from_string(const char *) noexcept;
    static const_string to_string(TmxLogLevel) noexcept;

    static bool can_log(TmxLogLevel, const char * = "") noexcept;
    static bool can_log(const char * = TMX_DEFAULT_LOG_LEVEL, const char * = "") noexcept;
    static void enable(TmxLogLevel, const char * = "") noexcept;
    static void enable(const char * = TMX_DEFAULT_LOG_LEVEL, const char * = "") noexcept;
    static void disable(const char * = "") noexcept;

    static void register_writer(TmxTypeDescriptor const &) noexcept;
    static void configure(types::Any const &) noexcept;

private:
    std::basic_ostringstream<char_t> _stream;

    std::chrono::system_clock::time_point const _time;
    std::string const _level;
    std::string const _nmspace;
    std::string const _file;
    std::uint64_t const _line;
};

// The TMX Log macro definition
#ifdef TMX_DISABLE_LOGGING
#define TMX_LOGGER(level)
#define TLOG(level)
#else
#define TMX_LOGGER(level) tmx::common::TmxLogger(level, TMX_PRETTY_FUNCTION, __FILE__, __LINE__).stream()
#define TLOG(level)                                                     \
    if (tmx::common::TmxLogger::can_log(#level, TMX_PRETTY_FUNCTION))   \
        TMX_LOGGER(#level)
#endif

// Backwards compatible alias for TMX plugins
#define PLOG(level) TMXLOG(level)

} /* End namespace common */
} /* End namespace tmx */

#endif /* LOGGING_INCLUDE_TMX_COMMON_TMXLOGGER_HPP_ */
