/*
 * Copyright (c) 2022 Battelle Memorial Institute
 *
 * All Rights Reserved.
 *
 * @file sizeof.hpp
 *
 *  Created on: Oct 5, 2022
 *      @author: gmb
 */

#ifndef PLATFORM_INCLUDE_TMX_COMMON_PLATFORM_TYPES_SIZE_OF_HPP_
#define PLATFORM_INCLUDE_TMX_COMMON_PLATFORM_TYPES_SIZE_OF_HPP_

#ifndef TMX_BITS_PER_BYTE
#define TMX_BITS_PER_BYTE 8
#endif

#include <cstddef>
#include <ratio>
#include <type_traits>

namespace tmx {
namespace common {

/*!
 * @brief A helper structure to store the size of the type
 *
 * This structure should be specialized for container types that
 * may be used during encoding or decoding.
 */
template <typename _Tp>
struct byte_size:
		public std::integral_constant<std::size_t, sizeof(_Tp)> { };

/*!
 * @return The number of bytes the type requires
 */
template <typename _Tp>
constexpr std::size_t byte_size_of() noexcept {
	return byte_size<_Tp>::value;
}

/*!
 * @return The number of bytes the type requires
 *
 * @param A reference to an instance of the type
 */
template <typename _Tp>
std::size_t byte_size_of(_Tp const &) noexcept {
	return byte_size_of<_Tp>();
}

/*!
 * @brief A helper structure to store the size of the type
 *
 * This structure should be specialized for container types that
 * may be used during encoding or decoding.
 */
template <typename _Tp>
struct bit_size:
		public std::integral_constant<std::size_t, byte_size_of<_Tp>() * TMX_BITS_PER_BYTE> { };

/*!
 * @return The number of bits the type requires
 */
template <typename _Tp>
constexpr std::size_t bit_size_of() noexcept {
	return bit_size<_Tp>::value;
}

/*!
 * @return The number of bits the type requires
 *
 * @param A reference to an instance of the type
 */
template <typename _Tp>
std::size_t bit_size_of(_Tp const &) noexcept {
	return bit_size_of<_Tp>();
}

/*!
 * @brief The total number of bytes for _Tp1 and _Tp2 as a std::ratio
 *
 * @see std::ratio
 */
template <typename _Tp1, typename _Tp2>
using byte_size_sum =
		typename std::ratio_add< std::ratio<bit_size_of<_Tp1>()>, std::ratio<byte_size_of<_Tp2>()> >;

/*!
 * @brief The difference between the number of bytes for _Tp1 and _Tp2 as a std::ratio
 *
 * This would be a negative ratio is _Tp1 has less bytes than _Tp2
 */
template <typename _Tp1, typename _Tp2>
using byte_size_difference =
		typename std::ratio_subtract< std::ratio<bit_size_of<_Tp1>()>, std::ratio<byte_size_of<_Tp2>()> >;

/*!
 * @brief Use the bigger type between _Tp1 and _Tp2
 */
template <typename _Tp1, typename _Tp2>
using bigger_byte_size =
		typename std::conditional<byte_size_difference<_Tp1, _Tp2>::num >= 0, _Tp1, _Tp2>::type;

/*!
 * @brief Use the smaller type between _Tp1 and _Tp2
 */
template <typename _Tp1, typename _Tp2>
using smaller_byte_size =
		typename std::conditional<byte_size_difference<_Tp1, _Tp2>::num <= 0, _Tp1, _Tp2>::type;

/*!
 * @brief The total number of bits for _Tp1 and _Tp2 as a std::ratio
 *
 * @see std::ratio
 */
template <typename _Tp1, typename _Tp2>
using bit_size_sum =
		typename std::ratio_add< std::ratio<bit_size_of<_Tp1>()>, std::ratio<bit_size_of<_Tp2>()> >;

/*!
 * @brief The difference between the number of bits for _Tp1 and _Tp2 as a std::ratio
 *
 * This would be a negative ratio is _Tp1 has less bits than _Tp2
 */
template <typename _Tp1, typename _Tp2>
using bit_size_difference =
		typename std::ratio_subtract< std::ratio<bit_size_of<_Tp1>()>, std::ratio<bit_size_of<_Tp2>()> >;

/*!
 * @brief Use the bigger type between _Tp1 and _Tp2
 */
template <typename _Tp1, typename _Tp2>
using bigger_bit_size =
		typename std::conditional<bit_size_difference<_Tp1, _Tp2>::num >= 0, _Tp1, _Tp2>::type;

/*!
 * @brief Use the smaller type between _Tp1 and _Tp2
 */
template <typename _Tp1, typename _Tp2>
using smaller_bit_size =
		typename std::conditional<bit_size_difference<_Tp1, _Tp2>::num <= 0, _Tp1, _Tp2>::type;


} /* End namespace common */
} /* End namespace tmx */


#endif /* PLATFORM_INCLUDE_TMX_COMMON_PLATFORM_TYPES_SIZE_OF_HPP_ */
