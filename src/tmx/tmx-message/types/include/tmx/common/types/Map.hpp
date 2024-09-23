/*!
 * Copyright (c) 2021 Battelle Memorial Institute
 *
 * All Rights Reserved.
 *
 * @file Map.hpp
 *
 *  Created on: Jul 21, 2021
 *      @author: Gregory M. Baumgardner
 */

#ifndef TYPES_INCLUDE_TMX_COMMON_TYPES_MAP_HPP_
#define TYPES_INCLUDE_TMX_COMMON_TYPES_MAP_HPP_

#include <tmx/platform.hpp>
#include <tmx/common/types/TmxDataType.hpp>
#include <tmx/common/types/String.hpp>

#include <cstdint>
#include <initializer_list>
#include <map>
#include <sstream>
#include <unordered_map>
#include <utility>

#ifndef TMX_HASH_ARRAY
#define TMX_HASH_ARRAY std::unordered_map
#endif

namespace tmx {
namespace common {
namespace types {

template <typename _Key>
using TmxHasher = typename TmxTypeOf<_Key>::hasher_type;

template <typename _Key, typename _Tp>
using TmxMapType = TMX_HASH_ARRAY<_Key, _Tp, TmxHasher<_Key> >;



/*
 * @brief A container for holding associative arrays
 *
 * TODO: Re-implement with Boost unordered map or some other better performing hash array
 */
template <typename _Key, typename _Tp>
class Map: public TmxDataType< TmxMapType<_Key, _Tp> > {
	/*!
	 * @brief The base class
	 */
	typedef TmxDataType< TmxMapType<_Key, _Tp> > super;

public:
	/*!
	 * @brief The value type
	 */
	typedef typename super::value_type value_type;

	typedef typename value_type::key_type key_t;
	typedef typename value_type::mapped_type value_t;
	typedef typename value_type::allocator_type allocator_t;

	/*!
	 * @brief A reflective type
	 */
	typedef Map<key_t, value_t> self_type;

	/*!
	 * @brief Construct an empty map
	 */
	Map(): super() { }

	/*!
	 * @brief Construct a map from the supplied STL container
	 *
	 * @param[in] value The map to use
	 */
	Map(value_type &&value): super(std::forward<value_type>(value)) { }

	/*!
	 * @brief Construct a map from the given reference
	 *
	 * @param[in] value The map to use
	 */
	Map(value_type &value): super(value) { }

	/*!
	 * @brief Construct a map from a constant map reference
	 *
	 * Note that this copies the sequence
	 *
	 * @param[in] value The map to use
	 */
	Map(value_type const &value): Map(value_type(value)) { }

	/*!
	 * @brief Construct a map from the supplied data
	 *
	 * Note that this copies the sequence
	 *
	 * @param[in] value The data to use
	 */
	Map(std::initializer_list<value_t> ilist):
			super(value_type(ilist)) { }

	/*!
	 * @brief Construct a map from iterators
	 *
	 * Note that this copies the sequence
	 *
	 * @param[in] first The first value to use
	 * @param[in] last The last value to use
	 */
	template <class InputIterator>
	Map(InputIterator first, InputIterator last):
			super(value_type(first, last)) { }

	using super::operator =;
	using super::operator *;
	using super::operator ->;

	/*!
	 * @return The number of bytes needed to store this TMX type
	 */
	std::size_t get_byte_count() const noexcept override {
		return (tmx::common::byte_size<key_t>::value +
				tmx::common::byte_size<value_t>::value) * this->size();
	}

	/*!
	 * @return The number of bits needed to store this TMX type
	 */
	std::size_t get_bit_count() const noexcept override {
		return (tmx::common::bit_size<key_t>::value +
				tmx::common::bit_size<value_t>::value) * this->size();
	}
};

/*!
 * @brief An alias name for the map type
 */
template <typename _Key, typename _Tp>
using map_type = Map<_Key, _Tp>;

/*!
 * @brief A special map to hold name-value properties
 */
template <typename _Tp = String_>
using Properties = Map< String_, _Tp >;

/*!
 * @brief An alias to basic properties;
 */
typedef Properties<> Properties_;

template <typename _Key, typename _Tp, bool = false, bool = false>
struct _map_name {
    static constexpr auto name = type_short_name< Map<_Key, _Tp> >();
};

template <typename _Key, typename _Tp>
struct _map_name<_Key, _Tp, true, false> {
private:
    static constexpr auto _name_keypart = introspect_short_name< TmxTypeOf<_Key> >();
    static constexpr auto _name_typepart = introspect_fqname<_Tp>();
    static constexpr auto _name_subpart = common::concat(_name_keypart,
                                                         common::concat(TMX_STATIC_STRING(", "), _name_typepart));
    static constexpr auto _name_front = common::concat(TMX_STATIC_STRING("Map<"), _name_subpart);
    static constexpr auto _name = common::concat(_name_front,
                                                 typename std::conditional<_name_front.array.back() == '>',
                                                 static_array<char, ' ', '>'>, static_array<char, '>'> >::type {});

public:
    static constexpr const_string name { _name.c_str(), _name.size };
};

template <typename _Key, typename _Tp>
struct _map_name<_Key, _Tp, false, true> {
private:
    static constexpr auto _name_keypart = introspect_fqname<_Key>();
    static constexpr auto _name_typepart = introspect_short_name< TmxTypeOf<_Tp> >();
    static constexpr auto _name_subpart = common::concat(_name_keypart,
                                                         common::concat(TMX_STATIC_STRING(", "), _name_typepart));
    static constexpr auto _name_front = common::concat(TMX_STATIC_STRING("Map<"), _name_subpart);
    static constexpr auto _name = common::concat(_name_front,
                                                 typename std::conditional<_name_front.array.back() == '>',
                                                         static_array<char, ' ', '>'>, static_array<char, '>'> >::type {});

public:
    static constexpr const_string name { _name.c_str(), _name.size };
};

template <typename _Key, typename _Tp>
struct _map_name<_Key, _Tp, true, true> {
private:
    static constexpr auto _name_keypart = introspect_short_name< TmxTypeOf<_Key> >();
    static constexpr auto _name_typepart = introspect_short_name< TmxTypeOf<_Tp> >();
    static constexpr auto _name_subpart = common::concat(_name_keypart,
                                                         common::concat(TMX_STATIC_STRING(", "), _name_typepart));
    static constexpr auto _name_front = common::concat(TMX_STATIC_STRING("Map<"), _name_subpart);
    static constexpr auto _name = common::concat(_name_front,
                                                 typename std::conditional<_name_front.array.back() == '>',
                                                         static_array<char, ' ', '>'>, static_array<char, '>'> >::type {});

public:
    static constexpr const_string name { _name.c_str(), _name.size };
};

template <typename _Key, typename _Tp>
using map_name = _map_name<_Key, _Tp, IsTmxType<_Key>::value, IsTmxType<_Tp>::value>;

template <typename _Key, typename _Tp>
struct TmxType<Map<_Key, _Tp>, void> {
	typedef std::true_type TMX;
	typedef Map<_Key, _Tp> type;
    static constexpr auto name = map_name<_Key, _Tp>::name;
};

template <typename _Key, typename _Tp>
struct TmxType<TmxMapType<_Key, _Tp>, void> {
	typedef std::true_type TMX;
	typedef Map<_Key, _Tp> type;
    static constexpr auto name = map_name<_Key, _Tp>::name;
};

} /* End namespace types */
} /* End namespace common */
} /* End namespace tmx */


#endif /* TYPES_INCLUDE_TMX_COMMON_TYPES_MAP_HPP_ */
