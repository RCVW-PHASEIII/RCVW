/*!
 * Copyright (c) 2021 Battelle Memorial Institute
 *
 * All Rights Reserved.
 *
 * @file String.hpp
 *
 *  Created on: Jul 19, 2021
 *      @author: Gregory M. Baumgardner
 */

#ifndef TYPES_INCLUDE_TMX_COMMON_TYPES_STRING_HPP_
#define TYPES_INCLUDE_TMX_COMMON_TYPES_STRING_HPP_

#include <tmx/platform.hpp>
#include <tmx/common/types/TmxDataType.hpp>

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <cuchar>
#include <initializer_list>
#include <string>
#include <utility>

namespace tmx {
namespace common {
namespace types {

template <class _C>
using string_char_t = typename _C::traits_type::char_type;

#ifndef TMX_CHAR8
#ifdef __cpp_char8_t
#define TMX_CHAR8 char8_t
#else
#define TMX_CHAR8 TMX_CHAR_TYPE
#endif
#endif
#ifndef TMX_CHAR16
#define TMX_CHAR16 char16_t
#endif
#ifndef TMX_CHAR32
#define TMX_CHAR32 char32_t
#endif

/*!
 * @brief Template structure for determining the minimum size of the type to use based on the number of bits
 */
template <std::uint8_t _Sz>
struct _char_container
{ typedef typename _char_container<_Sz - 1>::type type; };

// Template specializations
template <> struct _char_container<0>
{ typedef TMX_CHAR8 type; };
template <> struct _char_container<tmx::common::bit_size<TMX_CHAR8>::value + 1>
{ typedef TMX_CHAR16 type; };
template <> struct _char_container<tmx::common::bit_size<TMX_CHAR16>::value + 1>
{ typedef TMX_CHAR32 type; };

typedef typename _char_container<0>::type _charmin_t;
typedef typename _char_container<0xFF>::type _charmax_t;
typedef tmx::common::smaller_bit_size<tmx::common::char_t, _charmax_t> _char_t;

/*!
 * @brief A container for holding a string of characters
 */
template <std::uint8_t _Sz>
class String: public TmxDataType< std::basic_string<typename _char_container<_Sz>::type> > {
	/*!
	 * @brief The base class
	 */
	typedef TmxDataType< std::basic_string<typename _char_container<_Sz>::type> > super;

	static_assert(_Sz > 0,
			"The specified number of bits cannot be 0");
	static_assert(_Sz <= tmx::common::bit_size<_charmax_t>::value,
			"The specified number of bits is beyond a valid character size");
public:
	/*!
	 * @brief The value type
	 */
	typedef typename super::value_type value_type;

	/*!
	 * @brief A reflective type
	 */
	typedef String<_Sz> self_type;

	typedef typename value_type::value_type char_t;
	typedef typename value_type::traits_type traits_t;
	typedef typename value_type::allocator_type allocator_t;

	static_assert(tmx::common::is_integer<char_t>::value,
			"Container must contain an integral type");
	static_assert(tmx::common::is_string<value_type, char_t>::value,
			"The underlying value type must be a string type");

	typedef char_sequence<char_t> view_type;

	/*!
	 * @brief Construct an empty string
	 */
	String(): super() { }

	/*!
	 * @brief Construct a string in place
	 *
	 * @param[in] The string to use
	 */
	String(value_type &&value): super(std::forward<value_type>(value)) { }

	/*!
	 * @brief Construct a string from the given reference
	 *
	 * @param[in] The string to reference
	 */
	String(value_type &value): super(value) { }

	/*!
	 * @brief Construct a string from a constant string reference
	 *
	 * Note that this copies the character sequence
	 *
	 * @param[in] The string to use
	 */
	String(value_type const &value): String(value_type(value)) { }

	/*!
	 * @brief Construct a string from an existing character array
	 *
	 * Note that this copies the character sequence
	 *
	 * @param[in] The string to use
	 */
	String(const char_t *value, typename value_type::size_type len):
				String(value_type(value, len)) { }

	/*!
	 * @brief Construct a string from an existing character array
	 *
	 * Note that this copies the character sequence
	 *
	 * @param[in] The string to use
	 */
	String(const char_t *value):
				String(value_type(value, std::char_traits<char_t>::length(value))) { }

	/*!
	 * @brief Construct a string from an existing view
	 *
	 * Note that this copies the character sequence
	 *
	 * @param[in] The string to use
	 */
	String(view_type const &value):
				String(value_type(value.data(), value.length())) { }

	/*!
	 * @brief Construct a string from the supplied list of characters
	 *
	 * Note that this copies the character sequence

	 * @param[in] The initializer list of characters
	 */
	String(std::initializer_list<char_t> ilist):
				String(value_type(ilist)) { }

	/*!
	 * @brief Construct a string from a compile-time static string
	 *
	 * Note that this copies the character sequence
	 *
	 * @param[in] The string to use
	 */
	template <char... _V>
	String(tmx::common::static_array<char, _V...> const &s):
			super(value_type(s.c_str(), s.size)) { }

	using super::operator =;
	using super::operator *;
	using super::operator ->;

	/*!
	 * @brief The number of bytes needed to store this floating point type
	 */
	static constexpr auto numBytes =
			tmx::common::byte_size<char_t>::value;

	/*!
	 * @brief Only need _Sz number of bits to store the floating point
	 */
	static constexpr auto numBits = _Sz;

	/*!
	 * @return The number of bytes needed to store this TMX type
	 */
	std::size_t get_byte_count() const noexcept override {
		return numBytes * this->length();
	}

	/*!
	 * @return The number of bits needed to store this TMX type
	 */
	std::size_t get_bit_count() const noexcept override {
		return numBits * this->length();
	}

	/*!
	 * @return The contained string as a C-style character string
	 */
	operator const char_t *() const {
		return this->c_str();
	}

	/*!
	 * @return This string without any of the characters at the end
	 */
	value_type rtrim(const char *c = ws.c_str()) const noexcept {
		return ((value_type)*this).erase(this->find_last_not_of(c) + 1);
	}

	/*!
	 * @return This string without any of the characters at the beginning
	 */
	value_type ltrim(const char *c = ws.c_str()) const noexcept {
		return ((value_type)*this).erase(0, this->find_first_not_of(c));
	}

	/*!
	 * @return This string without any of the characters at the beginning or end
	 */
	value_type trim(const char *c = ws.c_str()) const noexcept {
		return self_type(this->rtrim(c)).ltrim(c);
	}

private:
	static constexpr tmx::common::whitespace_characters ws { };
};

typedef String<8>  String8;
typedef String<16> String16;
typedef String<32> String32;

template <typename _CharT = _char_t>
using string_type = String< tmx::common::bit_size<_CharT>::value >;

typedef string_type<_charmin_t> Stringmax;
typedef string_type<_charmax_t> Stringmin;
typedef string_type<> String_;

template <std::uint8_t _Sz>
struct TmxType<String<_Sz>, void> {
	typedef std::true_type TMX;
	typedef String<_Sz> type;
    static constexpr auto name = type_short_name<type>();
};

template <typename _CharT, class _Traits, class _Alloc>
struct TmxType<std::basic_string<_CharT, _Traits, _Alloc>, void> {
	typedef std::true_type TMX;
	typedef string_type<_CharT> type;
    static constexpr auto name = type_short_name<type>();
};

template <typename _CharT, class _Traits>
struct TmxType<TMX_CONST_STRING<_CharT, _Traits>, void> {
	typedef std::true_type TMX;
	typedef string_type<_CharT> type;
    static constexpr auto name = type_short_name<type>();
};

template <>
struct TmxType<const TMX_CHAR8 *, void> {
	typedef std::true_type TMX;
	typedef string_type<TMX_CHAR8> type;
    static constexpr auto name = type_short_name<type>();
};

template <>
struct TmxType<const TMX_CHAR16 *, void> {
	typedef std::true_type TMX;
	typedef string_type<TMX_CHAR16> type;
    static constexpr auto name = type_short_name<type>();
};

template <>
struct TmxType<const TMX_CHAR32 *, void> {
	typedef std::true_type TMX;
	typedef string_type<TMX_CHAR32> type;
    static constexpr auto name = type_short_name<type>();
};

using TmxStringTypes = TmxTypes< String<8>, String<16>, String<32> >;

template <typename _CharT>
string_type<_CharT> make_type(const _CharT *str) {
    if (!str) return { };
    return { str };
}

} /* End namespace types */
} /* End namespace common */
} /* End namespace tmx */

#endif /* TYPES_INCLUDE_TMX_COMMON_TYPES_STRING_HPP_ */
