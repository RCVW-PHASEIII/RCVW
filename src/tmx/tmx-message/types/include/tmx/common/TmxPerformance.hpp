/*
 * Copyright (c) 2023 Battelle Memorial Institute
 *
 * All Rights Reserved.
 *
 * @file TmxPerformance.hpp
 *
 *  Created on: Sep 18, 2023
 *      @author: Gregory M. Baumgardner
 */

#ifndef TYPES_INCLUDE_TMX_COMMON_TMXPERFORMANCE_HPP_
#define TYPES_INCLUDE_TMX_COMMON_TMXPERFORMANCE_HPP_

#include <tmx/platform.hpp>

#include <tmx/common/TmxLogger.hpp>

#include <chrono>

template <typename _Duration = std::chrono::duration<double, typename std::chrono::microseconds::period> >
class TmxTimer {
public:
    TmxTimer(const char *operation = "Operation"): _begin(std::chrono::steady_clock::now()), _op(operation) { }
    ~TmxTimer() {
        auto ts = std::chrono::duration_cast<_Duration>(std::chrono::steady_clock::now() - this->_begin);
        TLOG(DEBUG2) << this->_op << " completed in " << ts.count() << " " << units();
    }

private:
    std::chrono::steady_clock::time_point _begin;
    const char *_op;

    std::string units() {
        typedef typename _Duration::period ratio;
        if (ratio::num == 1) {
            switch (ratio::den) {
                case 1:
                    return "seconds";
                    break;
                case 1000:
                    return "milliseconds";
                    break;
                case 1000000:
                    return "microseconds";
                    break;
                case 1000000000:
                    return "nanoseconds";
                    break;
                default:
                    return "1/(" + std::to_string(ratio::den) + ") seconds";
            }
        } else {
            switch (ratio::num) {
                case 60:
                    return "minutes";
                    break;
                case 3600:
                    return "hours";
                    break;
                default:
                    return "(" + std::to_string(ratio::num) + ") seconds";
            }
        }
    }
};

#endif /* TYPES_INCLUDE_TMX_COMMON_TMXPERFORMANCE_HPP_ */
