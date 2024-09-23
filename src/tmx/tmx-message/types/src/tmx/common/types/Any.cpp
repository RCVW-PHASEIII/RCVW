/*
 * Copyright (c) 2023 Battelle Memorial Institute
 *
 * All Rights Reserved.
 *
 * @file Any.hpp
 *
 *  Created on: Mar 28, 2023
 *      @author: Gregory M. Baumgardner
 */

#include <tmx/common/TmxTypeRegistry.hpp>
#include <tmx/common/types/Any.hpp>

using namespace tmx::common;

namespace tmx {
namespace common {
namespace types {

TmxTypeDescriptor contents(Any const &data, const_string nmspace) noexcept {
    if (!data.has_value() || data.type() == typeid(void))
        return TmxTypeRegistry().get("null");

    return TmxTypeRegistry(typename TmxTypeRegistry::string(nmspace)).get(data.type());
}

bool contains(Any const &data, std::type_info const &id, const_string nmspace) noexcept {
    auto _desc = TmxTypeRegistry(typename TmxTypeRegistry::string(nmspace)).get(id);

    // Registered types can match by name or id
    if (_desc)
        return _desc.get_typeid() == data.type() ||
               _desc.get_type_name() == contents(data, nmspace).get_type_name();
    else
        return data.type() == id;
}

bool contains_tmx(Any const &data) noexcept {
    return TmxTypeRegistry().get(data.type());
}

} /* End namespace types */
} /* End namespace common */
} /* End namespace tmx */

