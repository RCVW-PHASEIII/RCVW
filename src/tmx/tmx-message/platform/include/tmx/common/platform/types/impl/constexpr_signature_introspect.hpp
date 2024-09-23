/*
 * Copyright (c) 2022 Battelle Memorial Institute
 *
 * All Rights Reserved.
 *
 * @file constexpr_signature_introspect.hpp
 *
 *  Created on: Oct 10, 2022
 *      @author: gmb
 */

#ifndef PLATFORM_INCLUDE_TMX_COMMON_PLATFORM_TYPES_IMPL_CONSTEXPR_SIGNATURE_INTROSPECT_HPP_
#define PLATFORM_INCLUDE_TMX_COMMON_PLATFORM_TYPES_IMPL_CONSTEXPR_SIGNATURE_INTROSPECT_HPP_

#include <tmx/common/platform/types/arrays.hpp>
#include <tmx/common/platform/types/introspect.hpp>

#ifndef TMX_INTROSPECTION_VERSION
#define TMX_INTROSPECTION_VERSION 1
#endif

namespace tmx {
namespace common {


/*!
 * @brief The static character array type equivalent for the fully-qualified name of _T
 *
 * This directly converts the constant character sequence name into
 * a static array structure of the characters in the string, but
 * only works with character arrays that are constexpr qualified.
 *
 * Since the type of this value is the resulting static character array
 * structure, with the characters of the name encoded in the variadic
 * template, it can be used to concatenate strings at compile time, if
 * necessary.
 *
 * @see #name
 * @see #concat
 * @return The static character array type name of _T
 */
template <typename _T>
constexpr auto _fqname(introspection_t<_T, 1> const &) noexcept {
	typedef introspection_t<_T, 1> type;

	return TMX_STATIC_STRING_VAR_SUBSTR( type::name_s, type::name.length() );
}

} /* End namespace common */
} /* End namespace tmx */


#endif /* PLATFORM_INCLUDE_TMX_COMMON_PLATFORM_TYPES_IMPL_CONSTEXPR_SIGNATURE_INTROSPECT_HPP_ */
