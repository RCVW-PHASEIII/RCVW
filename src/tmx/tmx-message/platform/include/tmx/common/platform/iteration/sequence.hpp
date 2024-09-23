/*!
 * Copyright (c) 2022 Battelle Memorial Institute
 *
 * All Rights Reserved.
 *
 * @file sequence.hpp
 *
 * Some general support functions for integer sequence operations
 *
 *  Created on: Mar 30, 2022
 *      @author: Gregory M. Baumgardner
 */

#ifndef PLATFORM_INCLUDE_TMX_COMMON_PLATFORM_ITERATION_SEQUENCE_HPP_
#define PLATFORM_INCLUDE_TMX_COMMON_PLATFORM_ITERATION_SEQUENCE_HPP_

#include <cstdint>
#include <tuple>
#include <utility>

namespace tmx {
namespace common {

template <class _T, _T... _I>
using integer_sequence = std::integer_sequence<_T, _I...>;

template <std::size_t ... _I>
using index_sequence = std::index_sequence<_I...>;

/*!
 * @brief Create a new index sequence from an existing one shifted left by the given number
 *
 * This is meant to be similar to the left shift (<<) operator except that it pulls in
 * the next numbers in the sequence. Thus:
 *
 * 0 1 2 3 4 5
 *
 * shifted left by 12 would produce:
 *
 * 12 13 14 15 16 17
 *
 * @param The index sequence to copy
 * @return A new index sequence with the values shifted by the given number
 */
template <std::size_t _N, typename _T, _T... _I>
constexpr auto leftshift_sequence(std::integer_sequence<_T, _I...> const &) noexcept {
	return std::integer_sequence<_T, static_cast<_T>(_N + _I)...> {};
}

/*!
 * @brief Create a new index sequence from an existing one shifted right by the given number
 *
 * This is meant to be similar to the right shift (>>) operator except that it pulls in
 * the next numbers in the sequence. Thus:
 *
 * 5 6 7 8 9 10
 *
 * shifted right by 2 would produce:
 *
 * 3 4 5 6 7 8
 *
 * @param The index sequence to copy
 * @return A new index sequence with the values shifted by the given number
 */
template <std::size_t _N, typename _T, _T... _I>
constexpr auto rightshift_sequence(std::integer_sequence<_T, _I...> const &) noexcept {
	return std::integer_sequence<_T, static_cast<_T>(_I - _N)...> { };
}

/*!
 * @brief A helper function to construct a sequence of the given length at the specified start
 *
 * The start position is defaulted to 0, but using this is the same as making the standard
 * index sequence. Instead, this structure is intended to create a sequence of a certain length
 * left-shifted by the specified amount. For example, constructing a sequence of length 5 from
 * the start position 9 would produce:
 *
 * 9 10 11 12 13
 *
 * @tparam _T The type of the integers to use
 * @tparam _Len The length of the sequence
 * @tparam _Start The first number of the sequence
 */template <class _T, _T _Len, _T _Start = 0>
constexpr auto make_integer_sequence() noexcept {
    return leftshift_sequence<_Start>(std::make_integer_sequence<_T, _Len> { });
}

/*!
 * @brief A helper function to construct a sequence of the given length at the specified start
 *
 * The start position is defaulted to 0, but using this is the same as making the standard
 * index sequence. Instead, this structure is intended to create a sequence of a certain length
 * left-shifted by the specified amount. For example, constructing a sequence of length 5 from
 * the start position 9 would produce:
 *
 * 9 10 11 12 13
 *
 * @param _Len The length of the sequence
 * @param _Start The first number of the sequence
 */
template <std::size_t _Len, std::size_t _Start = 0>
constexpr auto make_index_sequence() noexcept {
	return make_integer_sequence<std::size_t, _Len, _Start>();
}

/*!
 * @brief Find the first index of the given type in the list at compile-time
 *
 * Note this returns an out-of-range value if the type is not in the list.
 *
 * @tparam _T The type to look for
 * @tparam _I The index
 * @tparam _Tp
 * @return
 */
template <typename _T, std::size_t _I = 0, template <typename...> class _C, typename... _Tp>
constexpr std::enable_if_t<(_I < sizeof...(_Tp)), std::size_t> index_of(_C<_Tp...> const &tp) noexcept {
    typedef typename std::tuple_element<_I, std::tuple<_Tp...> >::type type;
    constexpr auto same = std::is_same_v<_T, type>;
    return same ? _I : index_of<_T, _I+1, _Tp...>();
}

template <typename _T, std::size_t _I, template <typename...> class _C, typename... _Tp>
constexpr std::enable_if_t<(_I >= sizeof...(_Tp)), std::size_t> index_of(_C<_Tp...> const &tp) noexcept {
    return sizeof...(_Tp) + 1;
}

} /* End namespace common */
} /* End namespace tmx */


#endif /* PLATFORM_INCLUDE_TMX_COMMON_PLATFORM_ITERATION_SEQUENCE_HPP_ */
