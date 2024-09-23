/*!
 * Copyright (c) 2021 Battelle Memorial Institute
 *
 * All Rights Reserved.
 *
 * @file Array.hpp
 *
 *  Created on: Jul 19, 2021
 *      @author: Gregory M. Baumgardner
 */

#ifndef TYPES_INCLUDE_TMX_COMMON_TYPES_ARRAY_HPP_
#define TYPES_INCLUDE_TMX_COMMON_TYPES_ARRAY_HPP_

#include <tmx/platform.hpp>
#include <tmx/common/types/TmxDataType.hpp>

#include <array>
#include <cstddef>
#include <initializer_list>
#include <list>
#include <queue>
#include <sstream>
#include <type_traits>
#include <utility>
#include <vector>

namespace tmx {
namespace common {
namespace types {

template <typename _Tp>
class Array: public TmxDataType< std::vector<_Tp> > {	/*!
	 * @brief The base class
	 */
	typedef TmxDataType< std::vector<_Tp> > super;

public:
	/*!
	 * @brief The value type
	 */
	typedef typename super::value_type value_type;

	/*!
	 * @brief A reflective type
	 */
	typedef Array<value_type> self_type;

	typedef typename value_type::value_type value_t;
	typedef typename value_type::allocator_type allocator_t;

	/*!
	 * @brief Construct an empty array
	 */
	Array() noexcept: super() { }

	/*!
	 * @brief Construct an array from the supplied STL container
	 *
	 * @param[in] value The array data to use
	 */
	Array(value_type &&value) noexcept: super(std::forward<value_type>(value)) { }

	/*!
	 * @brief Construct an array from the given reference
	 *
	 * @param[in] value The array to use
	 */
	Array(value_type &value) noexcept: super(value) { }

	/*!
	 * @brief Construct an array from a constant STL array reference
	 *
	 * Note that this copies the sequence
	 *
	 * @param[in] value The array data to use
	 */
	Array(value_type const &value) noexcept: Array(value_type(value)) { }

	/*!
	 * @brief Construct an array from the list of values
	 *
	 * Note that this copies the sequence
	 *
	 * @param[in] ilist The array values to use
	 */
	Array(std::initializer_list<value_t> ilist):
			super(value_type(ilist)) { }

	/*!
	 * @brief Construct an array from iterators
	 *
	 * Note that this copies the sequence
	 *
	 * @param[in] first The first value to use
	 * @param[in] last The last value to use
	 */
	template <class InputIterator>
	Array(InputIterator first, InputIterator last):
			super(value_type(first, last)) { }

	/*!
	 * @brief Construct an array from a STL array
	 *
	 * Note that this copies the sequence
	 *
	 * @param[in] The array to use
	 */
	template <typename _T, std::size_t _Sz>
	Array(std::array<_T, _Sz> const &a):
			Array(a.begin(), a.end()) { }

	/*!
	 * @brief Construct an array from a compile-time static array
	 *
	 * Note that this copies the sequence
	 *
	 * @param[in] The array to use
	 */
	template <typename _T, _T... _V>
	Array(tmx::common::static_array<_T, _V...> const &a):
			Array(a.array.begin(), a.array.end()) { }

	using super::operator =;
	using super::operator *;
	using super::operator ->;

	/*!
	 * @return The number of bytes needed to store this TMX type
	 */
	std::size_t get_byte_count() const noexcept override {
		return super::get_byte_count() * this->size();
	}

	/*!
	 * @return The number of bits needed to store this TMX type
	 */
	std::size_t get_bit_count() const noexcept override {
		return super::get_bit_count() * this->size();
	}
};

/*!
 * @brief An alias name for the array type
 */
template <typename _Tp>
using array_type = Array<_Tp>;

template <typename _Tp, bool = false>
struct _array_name {
    static constexpr auto name = type_short_name< Array<_Tp> >();
};

template <typename _Tp>
struct _array_name<_Tp, true> {
private:
    static constexpr auto _name_subpart = introspect_short_name< TmxTypeOf<_Tp> >();
    static constexpr auto _name_front = common::concat(TMX_STATIC_STRING("Array<"), _name_subpart);
    static constexpr auto _name = common::concat(_name_front,
                                                 typename std::conditional<_name_front.array.back() == '>',
                                                         static_array<char, ' ', '>'>, static_array<char, '>'> >::type {});

public:
    static constexpr const_string name { _name.c_str(), _name.size };
};

template <typename _Tp>
using array_name = _array_name<_Tp, IsTmxType<_Tp>::value>;

template <typename _Tp>
struct TmxType<Array<_Tp>, void> {
	typedef std::true_type TMX;
	typedef Array<_Tp> type;
    static constexpr auto name = array_name<_Tp>::name;
};

template <typename _Tp, typename _Alloc>
struct TmxType<std::vector<_Tp, _Alloc>, void> {
	typedef std::true_type TMX;
	typedef Array<_Tp> type;
    static constexpr auto name = array_name<_Tp>::name;
};

template <typename _Tp, std::size_t _Sz>
struct TmxType<std::array<_Tp, _Sz>, void> {
	typedef std::true_type TMX;
	typedef Array<_Tp> type;
    static constexpr auto name = array_name<_Tp>::name;
};

template <typename _Tp, typename _Alloc>
struct TmxType<std::list<_Tp, _Alloc>, void> {
	typedef std::true_type TMX;
	typedef Array<_Tp> type;
    static constexpr auto name = array_name<_Tp>::name;
};

template <typename _Tp, typename _Seq>
struct TmxType<std::queue<_Tp, _Seq>, void> {
	typedef std::true_type TMX;
	typedef Array<_Tp> type;
    static constexpr auto name = array_name<_Tp>::name;
};

template <typename _Tp, _Tp ... _V>
struct TmxType<tmx::common::static_array<_Tp, _V...>, void> {
	typedef std::true_type TMX;
	typedef Array<_Tp> type;
    static constexpr auto name = array_name<_Tp>::name;
};

template <typename>
struct _array_types;

template <typename... _Tp>
struct _array_types< std::tuple<_Tp...> > {
    typedef std::tuple< Array<_Tp>... > types;
};

template <typename _Tp>
using TmxArrayTypes = typename _array_types<_Tp>::types;

} /* End namespace types */
} /* End namespace common */
} /* End namespace tmx */

#endif /* TYPES_INCLUDE_TMX_COMMON_TYPES_ARRAY_HPP_ */
