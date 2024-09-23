/*
 * SampleData.cpp
 *
 *  Created on: May 16, 2016
 *      Author: ivp
 */

#include "SampleData.h"

#include <tmx/message/codec/TmxCodec.hpp>

using namespace std;
using namespace tmx::common::types;

namespace tmx {
namespace plugin {
namespace example {

// This class is used to demonstrate how code could be shared between
// a plugin (in the src folder) and the unit tests (in the test folder).
// See header file.

SampleData::SampleData() : _sequence(0), _timer(0) { }

SampleData::SampleData(Any const &memento) {
    auto props = as< Properties<Any> >(memento);
    if (props) {
        if (props->count("sequence")) {
            auto val = as< decltype(this->_sequence) >(props->at("sequence"));
            if (val) {
                this->set_sequence(val.value());
            } else {
                auto sVal = as<typename std::make_signed<decltype(this->_sequence)>::type>(props->at("sequence"));
                if (sVal) {
                    this->set_sequence(static_cast<decltype(this->_sequence)>(sVal.value()));;
                }
            }
        }

        if (props->count("timer")) {
            auto val = as< decltype(this->_timer) >(props->at("timer"));
            if (val) {
                this->set_timer(val.value());
            } else {
                auto sVal = as<typename std::make_signed<decltype(this->_timer)>::type>(props->at("timer"));
                if (sVal) {
                    this->set_timer(static_cast<decltype(this->_timer)>(sVal.value()));;
                }
            }
        }
    }
}

SampleData::~SampleData() { }

void SampleData::set_sequence(uint64_t val) {
    this->_sequence = val;
}

void SampleData::increment_sequence(uint64_t val) {
    this->_sequence += val;
}

uint64_t SampleData::get_sequence() const {
    return this->_sequence;
}

void SampleData::set_timer(uint64_t val) {
    this->_timer = val;
}

void SampleData::increment_timer(uint64_t val) {
    this->_timer += val;
}

uint64_t SampleData::get_timer() const {
    return this->_timer;
}

double SampleData::get_latency() const {
    if (this->_sequence)
        return 0.000001 * this->_timer / this->_sequence;

    return 0.0;
}

Any SampleData::get_memento() const {
    return Properties<Any>({ {"sequence", this->get_sequence()}, { "timer", this->get_timer() } });
}

} /* End namespace example */
} /* End namespace plugin */
} /* End namespace tmx */