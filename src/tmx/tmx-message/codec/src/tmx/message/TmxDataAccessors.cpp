/*!
 * Copyright (c) 2023 Battelle Memorial Institute
 *
 * All Rights Reserved.
 *
 * @file TmxDataAccessors.cpp
 *
 *  Created on: May 10, 2023
 *      @author: Gregory M. Baumgardner
 */

#include <tmx/message/TmxData.hpp>

using namespace tmx::common::types;

namespace tmx {
namespace message {

TmxData TmxData::operator[](UIntmax const &index) const noexcept {
    if (this->is_array()) {
        auto i = (typename UIntmax::value_type) index;

        // Read-only version
        const array_type *arr = tmx::common::any_cast<array_type>(&(this->get_container()));
        if (arr && i < arr->size()) {
            const Any &ref = arr->at(i);
            return { ref };
        }

        const typename array_type::value_type *varr =
                tmx::common::any_cast<typename array_type::value_type>(&(this->get_container()));
        if (varr && i < varr->size()) {
            const Any &ref = varr->at(i);
            return { ref };
        }
    }

    return { };
}

TmxData TmxData::operator[](UIntmax const &index) noexcept {
    auto i = (typename UIntmax::value_type)index;

    // Read/Write version
    array_type *arr = tmx::common::any_cast<array_type>(&(this->get_container()));
    typename array_type::value_type *varr =
            tmx::common::any_cast<typename array_type::value_type>(&(this->get_container()));
    if (!arr && !varr)
        arr = &(this->get_container().emplace<array_type>());

    Any null { Null() };
    if (arr && arr->size() <= i)
        arr->resize(i + 1, null);
    else if (varr && varr->size() <= i)
        varr->resize(i + 1, null);

    Any &ref = arr ? arr->at(i) : varr->at(i);
    return { ref };
}

TmxData TmxData::operator[](const common::types::String8 &key) const noexcept {
    // Read-only version
    if (this->is_map()) {
        const properties_type *props = tmx::common::any_cast<properties_type>(&(this->get_container()));
        if (props && props->count(key)) {
            const Any &ref = props->at(key);
            return { ref };
        }

        const typename properties_type::value_type *vprops =
                tmx::common::any_cast<typename properties_type::value_type>(&(this->get_container()));
        if (vprops && vprops->count(key)) {
            const Any &ref = vprops->at(key);
            return { ref };
        }
    }

    return { };
}

TmxData TmxData::operator[](typename String8::char_t const *key) const noexcept {
    return this->operator[](String8(key));
}

TmxData TmxData::operator[](const common::types::String8 &key) noexcept {
    // Read-write version
    properties_type *props = tmx::common::any_cast<properties_type>(&(this->get_container()));
    typename properties_type::value_type *vprops =
            tmx::common::any_cast<typename properties_type::value_type>(&(this->get_container()));
    if (!props && !vprops)
        props = &(this->get_container().emplace<properties_type>());

    if (props && !props->count(key))
        props->emplace(key, Null());
    else if (vprops && !vprops->count(key))
        vprops->emplace(key, Null());

    Any &ref = props ? props->at(key) : vprops->at(key);
    return { ref };
}

TmxData TmxData::operator[](typename String8::char_t const *key) noexcept {
    return this->operator[](String8(key));
}

} /* End namespace message */
} /* End namespace tmx */