/*!
 * Copyright (c) 2023 Battelle Memorial Institute
 *
 * All Rights Reserved.
 *
 * @file TmxDaoTraits.hpp
 *
 *  Created on: Mar 2, 2023
 *      @author: Gregory M. Baumgardner
 */

#ifndef DAO_INCLUDE_TMX_PLUGIN_DAO_TMXDAOTRAITS_HPP_
#define DAO_INCLUDE_TMX_PLUGIN_DAO_TMXDAOTRAITS_HPP_

#include <tmx/platform.hpp>

#include <tmx/common/TmxError.hpp>
#include <tmx/common/types/Any.hpp>

#include <type_traits>

namespace tmx {
namespace plugin {
namespace dao {

template <class _C>
using _decay = typename std::decay<_C>::type;

template <class _C>
using IsTmxClass = std::is_class< _decay<_C> >;

template <class _C>
using IsTmxAny = std::integral_constant<bool, IsTmxClass<_C>::value &&
        std::is_same<_C, common::types::Any>::value>;

template <class _C>
using IsTmxAnyConstructable = std::integral_constant<bool, IsTmxClass<_C>::value &&
        std::is_constructible<_decay<_C>, common::types::Any const &>::value>;

template <class _C>
using IsTmxAnyAssignable = std::integral_constant<bool, IsTmxClass<_C>::value &&
        std::is_assignable<_decay<_C>, common::types::Any const &>::value>;

template <class _C>
using IsTmxAnyConvertible = std::integral_constant<bool, IsTmxClass<_C>::value &&
        std::is_convertible<_decay<_C>, common::types::Any>::value>;

template <class _C>
using IsTmxDao = std::integral_constant<bool,
        IsTmxAnyConstructable<_C>::value && IsTmxAnyAssignable<_C>::value && IsTmxAnyConvertible<_C>::value>;

template <class _C>
typename std::enable_if<IsTmxAnyConstructable<_C>::value, _C>::type
make_dao(common::types::Any const &copy) {
    // Return the moved constructed object
    return { copy };
}

template <class _C>
typename std::enable_if<!IsTmxAnyConstructable<_C>::value && IsTmxAnyAssignable<_C>::value, _C>::type
make_dao(common::types::Any const &copy) {
    // Return a copy of the assigned object
    _C _tmp;
    _tmp = copy;
    return { _tmp };
}

template <class _C>
typename std::enable_if<!IsTmxAnyConstructable<_C>::value && !IsTmxAnyAssignable<_C>::value, _C>::type
make_dao(common::types::Any const &copy) {
    // Return a copy of the object, if the any type contains one
    auto const &_ptr = common::types::as<_C>(copy);
    if (_ptr)
        return { *_ptr };
    else
        return { };
}

template <typename _Tp>
typename std::enable_if<IsTmxAny<_Tp>::value, common::const_string>::type
get_type_schema(_Tp const &obj) {
    return obj.contents();
}

template <typename _Tp>
typename std::enable_if<!IsTmxAny<_Tp>::value && IsTmxAnyConvertible<_Tp>::value, common::const_string>::type
get_type_schema(_Tp const &obj) {
    return get_type_schema(static_cast<common::types::Any>(obj));
}



} /* End namespace dao */
} /* End namespace plugin */
} /* End namespace tmx */

#endif /* UTILS_INCLUDE_TMX_PLUGIN_UTILS_ASYNC_TMXRUNNABLE_HPP_ */
