/*
 * Logger.cpp
 *
 *  Created on: Jul 26, 2017
 *      Author: gmb
 */

#define FILE_NAME_MAX_WIDTH 32

#include "Logger.h"

#include "Clock.h"

#include <atomic>
#include <cstring>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <map>
#include <sys/time.h>
#include <tmx/messages/auto_message.hpp>

namespace tmx {
namespace utils {

std::ostream & _logtime(std::ostream &os, uint64_t timestamp);
std::ostream & _logsource(std::ostream &os, std::string file, uint32_t line);
std::ostream & _loglevel(std::ostream &os, LogLevel level);

// The default output log module
class FileLogger: public LogOutputModule {
public:
	const char *Name() noexcept;
	void Setup();
	void Teardown();

	void Output(LogLevel level, std::string component, const char *message);
	void Configure(tmx::message cfg);
private:
	std::atomic<uintptr_t> _stream;
	std::string _file;

	std::ostream *GetStream();
};

static std::list<std::shared_ptr<LogOutputModule> > _outputs;
static bool logOutputsInitialized = false;

Logger::Logger() {
	os.setf(std::ios::boolalpha);

	if (!logOutputsInitialized) {
		_outputs.resize(tmx::module::TmxModuleSize());
		size_t cnt = tmx::module::AllModules<LogOutputModule>(_outputs.begin());
		_outputs.resize(cnt);
		logOutputsInitialized = true;
	}
}

Logger::~Logger() {
	// Spit out to all the output modules
	for (auto module: _outputs) {
		if (module && module->IsEnabled() && this->level <= ReportingLevel())
			module->Output(this->level.Value(), this->component, os.str().c_str());
	}
}

LogLevel &Logger::MaxLevel() {
	static LogLevel _max = tmx::Enum<LogLevel>::AllValues().back().Value();
	return _max;
}


std::ostream &Logger::Get(LogLevel level, std::string file, unsigned int line, std::string component)
{
	this->level = level;
	this->component = component;

	_logtime(os, Clock::GetMillisecondsSinceEpoch());
	_logsource(os, file, line);
	_loglevel(os, level);
	return os;
}

std::string Logger::ToString(LogLevel level)
{
    return Enum<LogLevel>(level).Name();
}

LogLevel Logger::FromString(const char *level)
{
	Enum<LogLevel> e(level);
	if (!e.Valid()) e = DEFAULT_LOG_LEVEL;
	if (!e.Valid()) e = 0;
	return *e;
}

LogLevel Logger::FromString(const std::string &level)
{
	return Logger::FromString(level.c_str());
}

std::ostream &_logtime(std::ostream &os, uint64_t timestamp)
{
	struct timeval tv;

	if (timestamp == 0)
	{
		tv.tv_sec = timestamp / 1000;
		tv.tv_usec = timestamp % 1000 * 1000;
	}
	else
	{
		gettimeofday(&tv, NULL);
	}

	time_t time = tv.tv_sec;
	short ms = tv.tv_usec / 1000;

	struct tm *myTm = localtime(&time);
	char tmBuffer[20];
	strftime(tmBuffer, 20, "%F %T", myTm);

	os << "[" << tmBuffer << "." << std::setfill('0') << std::setw(3) << ms << "] " << std::setfill(' ');

	return os;
}

std::ostream &_logsource(std::ostream &os, std::string file, uint32_t line)
{
	static constexpr int fileMaxLen = FILE_NAME_MAX_WIDTH;

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

std::ostream &_loglevel(std::ostream &os, LogLevel level)
{
	os << " - " << std::left << std::setw(7) << Logger::ToString(level) << ": ";
	return os;
}

const char *FileLogger::Name() noexcept { return FILE_LOGGER; }

std::ostream *FileLogger::GetStream() {
	uintptr_t sPtr = _stream;
	if (sPtr > 0)
		return reinterpret_cast<std::ostream *>(sPtr);
	else
		return &std::cout;
}

void FileLogger::Setup() {
	if (!_file.empty()) {
		std::ofstream *fStream = new std::ofstream(_file, std::ios_base::app);
		_stream = reinterpret_cast<uintptr_t>(fStream);
	} else {
		_stream = 0;
	}
}

void FileLogger::Teardown() {
	// Close any open file stream
	std::ofstream *fStream = dynamic_cast<std::ofstream *>(GetStream());
	if (fStream) {
		_stream = 0;
		delete fStream;
	}
}

void FileLogger::Configure(tmx::message cfg) {
	std::string oldFile = _file;
	_file = cfg.get_untyped("logFile", "");

	if (_file != oldFile) {
		this->Teardown();
		this->Setup();
	}
}

void FileLogger::Output(LogLevel level, std::string component, const char *message) {
	*(GetStream()) << message << std::endl;
	GetStream()->flush();
}

// Need to create and register the default output module
static tmx::module::TmxModuleFactory<FileLogger> _fileLogHelper;

}} /* Namespace tmx::utils */
