/*!
 * Copyright (c) 2023 Battelle Memorial Institute
 *
 * All Rights Reserved.
 *
 * @file TmxData.cpp
 *
 *  Created on: May 10, 2023
 *      @author: Gregory M. Baumgardner
 */

#include <tmx/message/TmxData.hpp>

#include <utility>

using namespace tmx::common;
using namespace tmx::common::types;

namespace tmx {
namespace message {

TmxData::TmxData() noexcept: _container(Null()), _reference(_container) { }

TmxData::TmxData(TmxData const &copy) noexcept: TmxData() { this->operator=(copy); }

TmxData::TmxData(Any const &data) noexcept: _container(data), _reference(_container) { }

TmxData::TmxData(Any &&data) noexcept: TmxData(const_cast<const Any &>(data)) { data.reset(); }

TmxData::TmxData(types::Any &data) noexcept: _container(Null()), _reference(data) { }

TmxData &TmxData::operator=(TmxData const &copy) noexcept {
    if (&(copy._reference) == &(copy._container)) {
        // Make a copy
        const Any &_tmp = copy._container;
        return this->operator=(_tmp);
    } else {
        // Keep the reference
        return this->operator=(copy._reference);
    }
}

TmxData &TmxData::operator=(Any const &data) noexcept {
    if (data.has_value()) {
        this->_container = data;
        this->_reference = this->_container;
    }
    return *this;
}

TmxData &TmxData::operator=(Any &&data) noexcept {
    this->_container.swap(data);
    this->_reference = this->_container;
    return *this;
}

TmxData &TmxData::operator=(Any &data) noexcept {
    this->_container = Null();
    this->_reference = data;
    return *this;
}

Any const &TmxData::get_container() const noexcept {
    return this->_reference;
}

Any &TmxData::get_container() noexcept {
    return this->_reference;
}

bool TmxData::is_empty() const noexcept {
    return !this->get_container().has_value() || tmx::common::types::as<Null>(this->get_container());
}

bool TmxData::is_simple() const noexcept {
    return !this->is_empty() && !this->is_array() && !this->is_map() && contains_tmx(this->get_container());
}

bool TmxData::is_array() const noexcept {
    return (bool) tmx::common::types::as<array_type>(this->get_container());
}

bool TmxData::is_map() const noexcept {
    return (bool) tmx::common::types::as<properties_type>(this->get_container());
}

typename TmxData::array_type TmxData::to_array() const {
    auto arr = tmx::common::types::as<array_type>(this->get_container());
    if (arr)
        return *arr;
    else
        return { };
}

typename TmxData::properties_type TmxData::to_map() const {
    auto props = tmx::common::types::as<properties_type>(this->get_container());
    if (props)
        return *props;
    else
        return { };
}

} /* End namespace message */
} /* End namespace tmx */