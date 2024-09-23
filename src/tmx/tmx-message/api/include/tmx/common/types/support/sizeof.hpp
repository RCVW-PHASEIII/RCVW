/*!
 * Copyright (c) 2021 Battelle Memorial Institute
 *
 * All Rights Reserved.
 *
 * @file sizeof.hpp
 *
 *  Created on: Jun 22, 2021
 *      @author: Gregory M. Baumgardner
 */

#ifndef INCLUDE_TMX_COMMON_TYPES_SIZEOF_HPP_
#define INCLUDE_TMX_COMMON_TYPES_SIZEOF_HPP_

#include <tmx/common/types/support/consteval.hpp>

#include <cinttypes>
#include <cstddef>
#include <limits>
#include <type_traits>

namespace tmx {
namespace common {
namespace types {
namespace support {

/*!
 * @brief Determine how many bytes is needed to store the type
 *
 * This is almost equivalent to the built-in C++ sizeof()
 * operation except that this function considers not the object
 * or reference type, which may actually be TMX type container,
 * but the actual underlying type that the container holds.
 *
 * @param The type to test
 * @return The number of bytes needed to hold the type
 */
template <typename _T>
static inline constexpr std::size_t byte_sizeof(_T const * = nullptr) {
	return sizeof(type_of<_T>);
}

/*!
 * @brief Determine how many bytes is needed to store the type
 *
 * This is almost equivalent to the built-in C++ sizeof()
 * operation except that this function considers not the object
 * or reference type, which may actually be TMX type container,
 * but the actual underlying type that the container holds.
 *
 * @param The type to test
 * @return The number of bytes needed to hold the type
 */
template <typename _T>
static inline constexpr std::size_t byte_sizeof(_T const &) {
	return byte_sizeof<_T>();
}

/*!
 * @brief Determine how many bits is needed to store the type
 *
 * This is almost always 8 bits times the byte size, except if
 * the type itself contains a static constant variable named
 * "bits", such as for some of the primitive TMX types, in
 * which case that explicit value is used.
 *
 * Note that the long double type is misleading on some compilers
 * because it seems to be full quadruple (128-bit) precision, but likely
 * uses extended precision which is generally only 80 bits. So,
 * double check that the given 8-byte float really supports a full
 * 128-bit precision.
 *
 * Note that a 128-bit precision floating point number has 15 bits
 * for the exponent, and assumes extended precision floating points
 * (like for float80) have the same storage size for the exponent,
 * but also has one additional bit in the mantissa to store the
 * unit value.
 *
 * All TMX floating points shall adhere to strict the IEEE 754 format,
 * that use an implied unit value (which is 1). Thus, the codec for
 * an extended precision value may be more complicated than other
 * floating point values.
 *
 * @param The type to test
 * @return The number of bits needed to hold the type
 */
template <typename _T>
static inline constexpr std::size_t bit_sizeof(_T const * = nullptr) {
	constexpr bool has_bits = TMX_CHECK_FOR_STATIC_CONSTEXPR(_T, bits);
	if constexpr (has_bits) {
		return _T::bits;
	} else {
		constexpr std::size_t bits = byte_sizeof<_T>() * 8;
		constexpr bool is_float128 = (bits == 128 &&
				std::is_floating_point_v<_T> &&
				std::numeric_limits<_T>::is_iec559);

		// If there are 128 - 15 bits for the mantissa, which should include
		// the unit value bit, then this type is true quadruple precision
		// Otherwise, add the number of exponent bits and the sign bits
		// to the mantissa, which counts the unit value
		if constexpr (is_float128)
			return std::numeric_limits<_T>::digits == (bits - 15) ? bits :
					std::numeric_limits<_T>::digits + 15 + 1;
		else
			return bits;
	}
}

/*!
 * @brief Determine how many bits is needed to store the type
 *
 * This is almost always 8 bits times the byte size, except if
 * the type itself contains a static constant variable named
 * "bits", such as for some of the primitive TMX types, in
 * which case that explicit value is used.
 *
 * Note that the long double type is misleading on some compilers
 * because it seems to be full quadruple (128-bit) precision, but likely
 * uses extended precision which is generally only 80 bits. So,
 * double check that the given 8-byte float really supports a full
 * 128-bit precision.
 *
 * Note that a 128-bit precision floating point number has 15 bits
 * for the exponent, and assumes extended precision floating points
 * (like for float80) have the same storage size for the exponent,
 * but also has one additional bit in the mantissa to store the
 * unit value.
 *
 * All TMX floating points shall adhere to strict the IEEE 754 format,
 * that use an implied unit value (which is 1). Thus, the codec for
 * an extended precision value may be more complicated than other
 * floating point values.
 *
 * @param The type to test
 * @return The number of bits needed to hold the type
 */
template <typename _T>
static inline constexpr std::size_t bit_sizeof(_T const &) {
	return bit_sizeof<_T>();
}

} /* End namespace support */
} /* End namespace types */
} /* End namespace common */
} /* End namespace tmx */



#endif /* INCLUDE_TMX_COMMON_TYPES_SIZEOF_HPP_ */
