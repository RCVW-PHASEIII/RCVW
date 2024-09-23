/*!
 * Copyright (c) 2021 Battelle Memorial Institute
 *
 * All Rights Reserved.
 *
 * Adapted from:
 * http://www.drdobbs.com/cpp/logging-in-c/201804215
 *
 * @file TmxLogger.hpp
 *
 *  Created on: Aug 12, 2021
 *      @author: gmb
 */

#ifndef API_INCLUDE_TMX_COMMON_TMXLOGGER_HPP_
#define API_INCLUDE_TMX_COMMON_TMXLOGGER_HPP_

#include <tmx/common/TmxData.hpp>

#include <tmx/common/types/enum_type.hpp>
#include <tmx/common/types/int_type.hpp>
#include <tmx/common/types/string_type.hpp>
#include <tmx/common/types/support/introspect.hpp>

#include <sstream>

namespace tmx {
namespace common {

enum class TmxLogLevel: uint8_t {
	off,
	error,
	warning,
	info,
	debug,
	debug1,
	debug2,
	debug3,
	debug4
};

#ifdef TMX_NO_LOGGING
#undef TMX_DEFAULT_LOG_LEVEL
#define TMX_DEFAULT_LOG_LEVEL off
#else
#ifndef TMX_DEFAULT_LOG_LEVEL
#define TMX_DEFAULT_LOG_LEVEL info
#endif
#endif

#define TMX_LOG(LEVEL, WRITER) \
	if (tmx::common::TmxLogger(WRITER).level() == tmx::common::TmxLogLevel::off) ; \
	else if (tmx::common::TmxLogger(WRITER).level() < tmx::common::TmxLogLevel::LEVEL) ; \
	else tmx::common::TmxLogger(WRITER).log(tmx::common::TmxLogLevel::LEVEL, __func__, __FILE__, __LINE__)

// A short cut to the default logger
#define TLOG(LEVEL) TMX_LOG(LEVEL, "")

// These are for backwards compatibility
#define FILE_LOG(LEVEL) TMX_LOG(LEVEL, "")
#define PLOG(LEVEL) TMX_LOG(LEVEL, "")

/*!
 * @brief An abstract base-class for writing logging messages out
 *
 * It is intended that there only be a single instance for
 * each writer, so the write operation must be thread-safe
 */
class TmxLogWriter {
public:
	/*!
	 * @brief Construct the base-class instance
	 *
	 * @param[in] The name of this writer
	 */
	TmxLogWriter(typename types::string::view_type);

	/*!
	 * @brief Destroy the base-class instance
	 */
	virtual ~TmxLogWriter() = default;

	/*!
	 * @brief Write the given log message out
	 *
	 * Because the message can be any TMX data, it is up to the
	 * writer implementation how to encode it for output. This
	 * also may vary based on the contents of the message.
	 *
	 * @param[in] The data to log
	 */
	virtual void write(const any &) = 0;
};

/*!
 * @brief The class used to write log and debug messages.
 *
 * The name of the writer to use for this logger instance
 * is passed in at construction time. The log level setting
 */
class TmxLogger: public std::ostringstream {
public:
	typedef TmxLogLevel log_level_t;

	/*!
	 * @brief Construct a logger for the given writer
	 *
	 * @param[in] The name of the writer
	 */
	TmxLogger(typename types::string::view_type = "");
	~TmxLogger();

	// No assignments or copies allowed
	TmxLogger(const TmxLogger &) = delete;
	TmxLogger &operator =(const TmxLogger &) = delete;

	/*!
	 * @return The current log level for the writer
	 */
	TmxLogLevel &level();

	/*!
	 * @return The configuration parameters for the writer
	 */
	any &config();

	/*!
	 * @brief Enable the log writer for messages up to the specified level
	 *
	 * Note that this sets the level for any code using the log writer
	 *
	 * @param[in] The log level to enable to
	 */
	void enable(typename types::string::view_type = TMX_CPP_QUOTE(TMX_DEFAULT_LOG_LEVEL));

	/*!
	 * @brief Disable this writer
	 *
	 * Note that this disables writing for any code using the log writer,
	 * thus it must be re-enabled manually
	 */
	void disable();

	/*!
	 * @brief Initialize the logger
	 *
	 * This is an optional function, but is intended to pre-format the
	 * output based on the given log level, function, filename and line number.
	 *
	 * @param[in] The level to log to
	 * @param[in] The function name generating the log
	 * @param[in] The source file name generating the log
	 * @param[in] The source line number generating the log
	 */
	std::ostream &log(TmxLogLevel = TmxLogLevel::TMX_DEFAULT_LOG_LEVEL,
			typename types::string8::view_type = "",
			typename types::string8::view_type = "",
			typename types::uint64::type = 0);

private:
	any _logData;
};

} /* End namespace common */
} /* End namespace tmx */


#endif /* API_INCLUDE_TMX_COMMON_TMXLOGGER_HPP_ */
