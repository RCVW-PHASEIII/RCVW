/*!
 * Copyright (c) 2021 Battelle Memorial Institute
 *
 * All Rights Reserved.
 *
 * @file byte_order.hpp
 *
 *  Created on: Jun 22, 2021
 *      @author: Gregory M. Baumgardner
 */

#ifndef PLATFORM_INCLUDE_TMX_COMMON_PLATFORM_ENDIANNESS_BYTE_ORDER_HPP_
#define PLATFORM_INCLUDE_TMX_COMMON_PLATFORM_ENDIANNESS_BYTE_ORDER_HPP_

#include <tmx/common/platform/endianness/endian.hpp>
#include <tmx/common/platform/types/introspect.hpp>

#include <cstdint>
#include <type_traits>

namespace tmx {
namespace common {

/*!
 * @brief A class for little endian byte ordering
 */
enum class little_endian : std::uint8_t {};

/*!
 * @brief A class for big endian byte ordering
 */
enum class big_endian : std::uint8_t {};

/*!
 * @brief A class for an unknown byte ordering
 */
enum class unknown_byte_order : std::uint8_t {};

/*!
 * @brief Network byte ordering is always big endian
 *
 * @see https://datatracker.ietf.org/doc/html/rfc1700
 */
using network_byte_order = big_endian;

/*!
 * @brief An incomplete byte ordering object
 *
 * Any value for _E that is outside the known endianness
 * values will result in a compile error
 */
template <std::size_t _E>
struct byte_order_t;

/*!
 * @brief The little endian byte ordering object
 */
template <>
struct byte_order_t<TMX_LITTLE_ENDIAN> { using type = little_endian; };

/*!
 * @brief The big endian byte ordering object
 */
template <>
struct byte_order_t<TMX_BIG_ENDIAN> { using type = big_endian; };

/*!
 * @brief Middle-endian byte ordering is not explicitly handled
 */
template <>
struct byte_order_t<TMX_PDP_ENDIAN> { using type = unknown_byte_order; };

/*!
 * @brief An type alias for the specific byte order enum
 */
template <std::size_t _E = TMX_BYTE_ORDER>
using byte_order = typename byte_order_t<_E>::type;

/*!
 * @brief Get the specified byte ordering object
 *
 * This defaults to the byte ordering of the system
 *
 * @return The byte ordering object for _E
 */
template <std::size_t _E = TMX_BYTE_ORDER>
constexpr auto get_byte_order() noexcept {
	return byte_order<_E> {};
}

/*!
 * @brief Get the specified byte ordering object name
 *
 * This defaults to the byte order of the system
 *
 * @return The byte ordering name for _E
 */
template <std::size_t _E = TMX_BYTE_ORDER>
constexpr const_string get_endianness() noexcept {
	return type_short_name(get_byte_order<_E>());
}

/*!
 * @return true_type if this system's byte_order is the same as _E, false_type otherwise
 */
template <std::size_t _E>
using check_byte_order = std::is_same< byte_order<_E>, byte_order<> >;

/*!
 * @return True if this system uses the specified byte ordering
 */
template <std::size_t _E>
constexpr bool is_byte_order() noexcept {
	return check_byte_order<_E>::value;
}

/*!
 * @return true_type if this system's byte order is little_endian, false_type otherwise
 */
using check_little_endian = check_byte_order<TMX_LITTLE_ENDIAN>;

/*!
 * @return True if this system uses little endian byte ordering
 */
constexpr bool is_little_endian() noexcept {
	return check_little_endian::value;
}

/*!
 * @return true_type if this system's byte order is big_endian, false_type otherwise
 */
using check_big_endian = check_byte_order<TMX_BIG_ENDIAN>;

/*!
 * @return True if this system uses big endian byte ordering
 */
constexpr bool is_big_endian() noexcept {
	return check_big_endian::value;
}

/*!
 * @return true_type if this system's byte order is network byte order, false_type otherwise
 */
using check_network_byte_order = check_byte_order<TMX_NETWORK_BYTE_ORDER>;

/*!
 * @return True if this system uses network byte ordering
 */
constexpr bool is_network_byte_order() noexcept {
	return check_network_byte_order::value;
}

} /* End namespace common */
} /* End namespace tmx */

#endif /* PLATFORM_INCLUDE_TMX_COMMON_PLATFORM_ENDIANNESS_BYTE_ORDER_HPP_ */
