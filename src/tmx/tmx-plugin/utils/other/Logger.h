/*
 * Logger.h
 *
 * Generic logger based on output modules
 *
 * Adapted from:
 * http://www.drdobbs.com/cpp/logging-in-c/201804215
 *
 * Logging In C++
 * By Petru Marginean, September 05, 2007
 *
 *
 *  Created on: Jul 26, 2017
 *      @uthor: Gregory M. Baumgardner
 */

#ifndef SRC_LOGGER_H_
#define SRC_LOGGER_H_

#include <sstream>
#include "TmxLog.h"
#include "TmxModule.h"
#include "tmx/messages/message.hpp"

#ifndef DEFAULT_LOG_LEVEL
#define DEFAULT_LOG_LEVEL "INFO"
#endif

#define UNKNOWN_SOURCE "Unknown source"

#define TMX_LOG(level, component) \
	if (tmx::utils::LogLevel::level > Logger::MaxLevel()) ; \
	else if (tmx::utils::LogLevel::level > tmx::utils::Logger::ReportingLevel()) ; \
	else tmx::utils::Logger().Get(tmx::utils::LogLevel::level, __FILE__, __LINE__, component)
#define FILE_LOG(level) TMX_LOG(level, "")

#define FILE_LOGGER "FileLogger"

namespace tmx {
namespace utils {

class LogOutputModule: public tmx::module::TmxModule {
public:
	virtual ~LogOutputModule() { }

	// Pure virtual functions to override
	virtual void Output(LogLevel level, std::string component, const char *message) = 0;
	virtual void Configure(tmx::message config) = 0;

	inline const char *Type() noexcept { return "Log writer"; }
};


class Logger {
public:
	Logger();
	~Logger();

	// No assignments or copies allowed
	Logger(const Logger&) = delete;
	Logger &operator=(const Logger &) = delete;

	// Conversions to/from string
    static std::string ToString(LogLevel level);
    static LogLevel FromString(const char *level);
    static LogLevel FromString(const std::string &level);

    // Other static functions
    static LogLevel &MaxLevel();
    static inline LogLevel &ReportingLevel() {
    	// Declared inline because the default level can vary program to program due to the macro constant
    	static LogLevel _level = FromString(DEFAULT_LOG_LEVEL);
    	return _level;
    }

	// The access operation
	std::ostream &Get(LogLevel level = FromString(DEFAULT_LOG_LEVEL), std::string file = UNKNOWN_SOURCE, uint32_t line = 0, std::string component = "");

protected:
	tmx::Enum<LogLevel> level;
	std::string component;
	std::ostringstream os;
};

}} /* Namespace tmx::utils */


#endif /* SRC_LOGGER_H_ */
