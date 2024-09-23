/*!
 * Copyright (c) 2021 Battelle Memorial Institute
 *
 * All Rights Reserved.
 *
 * @file TmxFileLogger.cpp
 *
 *  Created on: Aug 24, 2021
 *      @author: gmb
 */

#include <tmx/common/TmxLogger.hpp>

#include <algorithm>
#include <chrono>
#include <iostream>
#include <iomanip>
#include <fstream>

#define FILE_NAME_MAX_WIDTH 40

namespace tmx {
namespace common {

std::ostream &_logtime(std::ostream &os)
{
	auto now = std::chrono::system_clock::now().time_since_epoch();
	auto sec = std::chrono::duration_cast<std::chrono::seconds>(now);
	
	time_t time = sec.count();
	auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now - sec).count();

	struct tm myTm;
#ifdef _MSC_VER
	localtime_s(&myTm, &time);
#else
	localtime_r(&time, &myTm);
#endif


	char tmBuffer[20];
	strftime(tmBuffer, 20, "%F %T", &myTm);

	os << "[" << tmBuffer << "." << std::setfill('0') << std::setw(3) << ms << "] " << std::setfill(' ');

	return os;
}

std::ostream &_logsource(std::ostream &os, types::string function, types::string file, types::string line)
{
	static constexpr int fileMaxLen = FILE_NAME_MAX_WIDTH;

	if (line > 0)
	{
		file.append(" (");
		file.append(function);
		file.append(") (");
		file.append(line);
		file.append(")");
	}

	if (file.length() > fileMaxLen)
		os << std::right << std::setw(fileMaxLen) << file.substr(file.size() - fileMaxLen);
	else
		os << std::right << std::setw(fileMaxLen) << file;

	return os;
}

std::ostream &_loglevel(std::ostream &os, types::string level)
{
	os << " - " << std::left << std::setw(7) << level << ": ";
	return os;
}

class TmxFileLogWriter: public TmxLogWriter {
public:
	TmxFileLogWriter(typename types::string::view_type &&nm): TmxLogWriter(nm) { }
	~TmxFileLogWriter() {
		destroy();
	}

	void destroy() {
		if (fstream)
			delete fstream;
		fstream = nullptr;

		file = "-";
	}

	void write(const any &data) override {
		auto logData = data.to_map();
		const TmxData &cfg = logData["config"];

		std::ofstream *fstream = nullptr;

		// Obtain the file name
		types::string fileName = cfg["file"].to_string();
		if (fileName.empty() || fileName == "null")
			fileName = "-";

		if (fileName != file) {
			this->destroy();

			if (fileName != "-")
				fstream = new std::ofstream(file.c_str(), std::ios_base::app);

			file = fileName;
		}

		std::ostream *os = fstream;
		if (!os) os = &std::cout;

		_logtime(*os);
		_logsource(*os, logData["function"].to_string(),
						logData["filename"].to_string(),
						logData["line"].to_string());

		//typename enum_t::type lvl = logData["level"].to_int();
		//types::string lvlName { (const char *)enum_t(lvl) };
		//std::transform(lvlName.begin(), lvlName.end(), lvlName.begin(), ::toupper);
		_loglevel(*os, "TODO");

		*os << logData["message"];

		if (!logData["no-newline"])
			*os << std::endl;

		os->flush();
	}

	std::string file { "-" };
	std::ofstream *fstream = nullptr;
};

static TmxFileLogWriter _file_writer { "file" };
static TmxFileLogWriter _default_writer { "" };

} /* End namespace common */
} /* End namespace tmx */
