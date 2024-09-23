/*!
 * Copyright (c) 2021 Battelle Memorial Institute
 *
 * All Rights Reserved.
 *
 * @file TmxLogger.cpp
 *
 *  Created on: Aug 12, 2021
 *      @author: gmb
 */

#include <tmx/common/TmxLogger.hpp>

#include <tmx/common/TmxTypeRegistry.hpp>
#include <tmx/common/types/array_type.hpp>

namespace tmx {
namespace common {

class TmxLoggerInstance {
public:
	TmxLoggerInstance(TmxLogWriter *logWriter): writer(logWriter) { }

	auto &level() { return this->_level; }
	auto &config() { return this->_cfg; }

	TmxLogLevel _level = TmxLogLevel::off;

	TmxData _cfg;
	TmxLogWriter *writer = nullptr;
};

class LoggerRegistry: public TmxRegistry<TmxLoggerInstance, types::string> {
public:
	static constexpr auto default_key = types::null_type::name;

	static auto &instance() {
		static LoggerRegistry _singleton;
		return _singleton;
	}

	void registerWriter(typename types::string::view_type name, TmxLogWriter *ptr) {
		if (name.empty())
			name = default_key;

		if (ptr) {
			TmxLoggerInstance *newInstance = new TmxLoggerInstance(ptr);
			this->operator [](key_t(name)) = newInstance;
		}
	}

	TmxLoggerInstance *get(key_t &&key) {
		if (this->count(key))
			return this->operator [](key);

		return nullptr;
	}

	static TmxLoggerInstance *get(const any &data) {
		static TmxLoggerInstance _null(nullptr);

		TmxLoggerInstance *ret = instance().get(data["writer"].to_string());
		if (ret) return ret;
		return &_null;
	}
};

TmxLogWriter::TmxLogWriter(typename types::string::view_type name) {
	LoggerRegistry::instance().registerWriter(name, this);
}

TmxLogLevel &TmxLogger::level() {
	return LoggerRegistry::get(this->_logData)->level();
}

TmxData &TmxLogger::config() {
	return LoggerRegistry::get(this->_logData)->config();
}

TmxLogger::TmxLogger(typename types::string::view_type name): std::ostringstream() {
	// Save the writer name
	if (name.length())
		this->_logData["writer"] = name;
	else
		this->_logData["writer"] = LoggerRegistry::default_key;

	// Enable automatically on first use
	any &cfg = this->config();
	if (!cfg["accessed"]) {
		this->enable();

		cfg["accessed"] = true;
	}

	this->_logData["config"] = cfg;
}

TmxLogger::~TmxLogger() {
	// Write out the log message at destruction
	int l = (int)this->_logData["level"];

	TmxLoggerInstance *logger = LoggerRegistry::get(this->_logData);
	if (logger && logger->writer && l > 0 && (int)logger->level() >= l) {
		this->_logData["message"] = this->str();
		logger->writer->write(this->_logData);
	}
}

void TmxLogger::enable(typename types::string::view_type lvl) {
	//auto l = types::Enum::get_value<TmxLogLevel>(lvl);
	//if (l)
	//	this->level()  = *l;
}

void TmxLogger::disable() {
	this->enable("off");
}

std::ostream &TmxLogger::log(TmxLogLevel lvl,
		typename types::string::view_type function,
		typename types::string::view_type filename,
		typename types::uint64::type line) {
	//typedef typename log_level_t::type type;
	this->_logData["level"] = (int)(lvl);
	this->_logData["function"] = function;
	this->_logData["filename"] = filename;
	this->_logData["line"] = line;

	return *this;
}

} /* End namespace common */
} /* End namespace tmx */


