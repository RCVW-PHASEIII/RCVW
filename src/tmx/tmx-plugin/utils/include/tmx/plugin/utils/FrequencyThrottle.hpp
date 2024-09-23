/*
 * FrequencyRegulator.h
 *
 *  Created on: Jun 7, 2016
 *      Author: ivp
 */

#ifndef SRC_FREQUENCYTHROTTLE_H_
#define SRC_FREQUENCYTHROTTLE_H_

#include <chrono>
#include <map>

namespace tmx {
namespace plugin {
namespace utils {

/**
 * This class is used to monitor a data source so that it can be throttled.
 */
template <class _KeyType, typename _Clock = std::chrono::steady_clock>
class FrequencyThrottle
{
    typedef typename _Clock::duration duration;
public:
	FrequencyThrottle() {
        set_Frequency(duration(0));
    }

	/**
	 * @param frequency The frequency at which Monitor returns true.
	 */
	FrequencyThrottle(duration frequency) {
        set_Frequency(std::chrono::duration_cast<duration>(frequency));
    }

    virtual ~FrequencyThrottle() = default;

	/**
	 * Update the timestamp for the key specified and return true if the maxFrequency
	 * specified in the constructor has elapsed since the last time this method returned true
	 * for the matching key.
	 * Monitor will always return true the first time it is called for each unique key.
	 *
	 * @param key The unique key to monitor.
	 * @returns true if the frequency has elapsed since this method last returned true for the key.
	 */
	bool Monitor(_KeyType key) {
        auto now = _Clock::now();

        // Find the key in the map.
        auto it = this->_mapLastTime.find(key);

        // If key not found, store the current time, then return true to indicate it is time to do any processing.
        if (it == _mapLastTime.end()) {
            this->_mapLastTime[key] = now;
            return true;
        }

        // Determine duration since last time that true was returned.
        auto freq = this->get_Frequency();
        duration time = std::chrono::duration_cast<duration>(now - it->second);

        // If duration surpassed, store new time and return true.
        if (time >= freq) {
            it->second = now;
            return true;
        }

        return false;
    }

	/**
	 * Remove the data associated with any stale keys.
	 * This method should be called periodically if it is expected for keys to no longer be relevant.
	 */
	void RemoveStaleKeys() {
        auto now = _Clock::now();

        auto it = this->_mapLastTime.begin();

        auto staleDuration = duration(this->_stalePeriods);

        while (it != this->_mapLastTime.end()) {
            auto time = std::chrono::duration_cast<duration>(now - it->second);

            if (time >= staleDuration)
                 this->_mapLastTime.erase(it);

            it++;
        }
    }

	/**
	 * Get the frequency at which Monitor returns true.
	 */
    template <typename _Duration = duration>
	_Duration get_Frequency() {
        return std::chrono::duration_cast<_Duration>(this->_frequency);
    }

	/**
	 * Set the frequency at which Monitor returns true.
	 */
    template <typename _Duration>
	void set_Frequency(_Duration frequency) {
        this->_frequency = std::chrono::duration_cast<duration>(frequency);
    }

    void set_StaleDuration(std::uintmax_t periods) {
        this->_stalePeriods = periods;
    }

    template <typename _Duration>
    void set_StaleDuration(_Duration time) {
        this->set_StaleDuration(std::chrono::duration_cast<duration>(time).count());
    }

	/**
	 * Update the timestamp for the key specified in order to reset the clock.  Does
	 * nothing if the key is not found.
	 */
	void Touch(_KeyType key) {
        // Find the key in the map.
        auto it = this->_mapLastTime.find(key);

        // If key not found, do nothing
        if (it == this->_mapLastTime.end())
            return;

        // Update timestamp
        typename _Clock::time_point now = _Clock::now();
        it->second = now;
    }
private:
	duration _frequency;
	std::uintmax_t _stalePeriods { 5000 };
	std::map<_KeyType, typename _Clock::time_point> _mapLastTime;
};

} /* namespace utils */
} /* namespace plugin */
} /* namespace tmx */

#endif /* SRC_FREQUENCYTHROTTLE_H_ */
