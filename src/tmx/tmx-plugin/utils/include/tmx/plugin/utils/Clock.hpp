/*
 * Clock.h
 *
 *  Created on: Aug 30, 2016
 *      Author: ivp
 */

#ifndef SRC_CLOCK_H_
#define SRC_CLOCK_H_

#include <tmx/platform.hpp>

#include <chrono>
#include <string>
#include <ctime>
#include <ratio>

namespace tmx {
namespace plugin {
namespace utils {

class Clock {
public:
	/**
	 * Print information about the standard C++ clocks defined in std::chrono.
	 */
	static void PrintInfoForChronoClocks();

	/**
	 * Convert a time_point into a local calendar string.
	 * Example return format: Mon Sep 12 11:28:53 2016.
	 */
	static std::string ToLocalTimeString(const std::chrono::system_clock::time_point& tp);

	/**
	 * Convert a time_point into a UTC calendar string.
	 * Example return format: Mon Sep 12 11:28:53 2016.
	 */
	static std::string ToUtcTimeString(const std::chrono::system_clock::time_point& tp);


	static long getAdjustedTime(unsigned int offset_tenthofSec);

	/**
	 * Convert a time_point into a local calendar string using abbreviated format and fractional seconds.
	 * Example return format: 2016-09-12 11:28:53.773.
	 */
	static std::string ToLocalPreciseTimeString(const std::chrono::system_clock::time_point& tp);

	/**
	 * Convert a time_point into a UTC calendar string using abbreviated format and fractional seconds.
	 * Example return format: 2016-09-12 11:28:53.773.
	 */
	static std::string ToUtcPreciseTimeString(const std::chrono::system_clock::time_point& tp);

	/**
	 * Convert a time_point into a UTC calendar string.
	 * Example return format: 2017-02-27 21:23:32.773.
	 */
	static std::string ToUtcPreciseTimeString(uint64_t ms);
	/**
	 * Convert a chrono::milliseconds duration into a string formatted as hh:mm:ss.fff.
	 */
	static std::string ToTimeString(const std::chrono::milliseconds ms);

	/**
	 * Return the count of milliseconds since the epoch.
	 */
	static uint64_t GetMillisecondsSinceEpoch();

    /**
     * Return the count of milliseconds since the epoch.
     */
    static uint64_t GetMillisecondsSinceEpoch(struct timespec &);

    /**
	 * Return the count of milliseconds since the epoch of the specified time point
	 */
	static uint64_t GetMillisecondsSinceEpoch(const std::chrono::system_clock::time_point& tp);

	/**
	 * Return the specified milliseconds since the epoch into a std::chrono time point
	 */
	static std::chrono::system_clock::time_point GetTimepointSinceEpoch(uint64_t ms);

	/*
	 * Return the specified milliseconds since the epoch for a given time string, in UTC
	 */
	static uint64_t GetMillisecondsSinceEpoch(std::string timestring, std::string format = "%Y-%m-%d %H:%M:%S");
};

} /* namespace utils */
} /* namespace plugin */
} /* namespace tmx */

#endif /* SRC_CLOCK_H_ */
