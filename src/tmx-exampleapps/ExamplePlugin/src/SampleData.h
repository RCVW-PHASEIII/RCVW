/*
 * SampleData.h
 *
 *  Created on: May 16, 2016
 *      Author: ivp
 */

#ifndef SAMPLEDATA_H_
#define SAMPLEDATA_H_

#include <tmx/common/types/Any.hpp>

#include <cstdint>

namespace tmx {
namespace plugin {
namespace example {

/*!
 * @brief A Sample of data that may be transferred between TMX components
 *
 * This class contains a sequence counter and a time increment in order to
 * calculate the average latency.
 */
class SampleData {
public:
    SampleData();
    SampleData(common::types::Any const &);
    virtual ~SampleData();

    void set_sequence(std::uint64_t);
    void increment_sequence(std::uint64_t = 1);
    std::uint64_t get_sequence() const;

    void set_timer(std::uint64_t);
    void increment_timer(std::uint64_t = 1);
    std::uint64_t get_timer() const;

    double get_latency() const;

    common::types::Any get_memento() const;
private:
    std::uint64_t _sequence;
    std::uint64_t _timer;
};

} /* End namespace example */
} /* End namespace plugin */

} /* End namespace tmx */

#endif /* SAMPLEDATA_H_ */
