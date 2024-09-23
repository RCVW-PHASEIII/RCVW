/*!
 * Copyright (c) 2021 Battelle Memorial Institute
 *
 * All Rights Reserved.
 *
 * @file static_array.hpp
 *
 * This file contains constant expression evaluation types and
 * functions that are used in TMX.
 *
 * Note that
 *  Created on: Apr 9, 2022
 *      @author: Gregory M. Baumgardner
 */

#ifndef PLATFORM_INCLUDE_TMX_COMMON_PLATFORM_TYPES_ARRAYS_HPP_
#define PLATFORM_INCLUDE_TMX_COMMON_PLATFORM_TYPES_ARRAYS_HPP_

#include <tmx/common/platform/types/bytes.hpp>
#include <tmx/common/platform/types/traits.hpp>

#include <boost/hana.hpp>

#include <array>
#include <cstddef>
#include <cstdint>
#include <utility>

namespace tmx {
namespace common {

/*!
 * @brief An alias for the boost::hana implementation of a compile-time static array
 */
template <typename _T, _T ... _V>
using static_array_t = decltype(boost::hana::tuple_c<_T, _V...>);

/*!
 * @brief An alias for the boost::hana implementation of a compile-time static character array
 */
template <char ... _V>
using static_string_t = boost::hana::string<_V...>;

/*!
 * @brief The TMX static array structure is a compile-time view of integral constant values
 */
template <typename _T, _T ... _V>
struct static_array {
	static constexpr std::size_t size = sizeof...(_V);

	typedef _T value_type;
	typedef static_array_t<_T, _V...> type;
	static constexpr auto value = type {};

	static constexpr const std::array<_T, size> &array = boost::hana::unpack(value,
			[](auto && ...x) { return std::array<_T, size>{std::forward<decltype(x)>(x)...}; });

	static constexpr const _T *data() { return array.data(); }

	constexpr operator const _T *() const { return array.data(); }
	constexpr const auto &at(std::size_t i) const { return array.at(i); }
};

/*!
 * @brief The TMX static array for character values is a compile-time view of character constant values
 *
 * This is a special instance of the static_array because it can be converted to a constant string or
 * used like a string literal
 */
template <char ... _V>
struct static_array<char, _V...> {
	static constexpr std::size_t size = sizeof...(_V);

	typedef char value_type;
	typedef static_string_t<_V...> type;
	static constexpr auto value = type {};

	static constexpr std::array<char, size> array = boost::hana::unpack(value,
			[](auto && ...x) { return std::array<char, size>{std::forward<decltype(x)>(x)...}; });

	static constexpr const char *c_str() { return value.c_str(); }

	constexpr operator const char *() const { return value.c_str(); }
	constexpr const auto &at(std::size_t i) const { return array.at(i); }
};

/*!
 * @brief Helper function that determines if the given structure is a static character array
 *
 * @param The static array to test
 * @return True if the type is a static character array
 */
template <class _C>
static inline constexpr bool is_static_string( _C const * = nullptr ) {
	typedef type_unboxer<_C> type;
	return std::is_same_v<boost::hana::string_tag, boost::hana::tag_of_t<type> >;
}

/*!
 * @brief Helper function that determines if the given structure is a static character array
 *
 * @param The static array to test
 * @return True if the type is a static character array
 */
template <class _C>
static inline constexpr bool is_static_string( _C const & ) {
	return is_static_string<_C>();
}

/*!
 * @brief Factory method to create a new compile-time static array from an existing one
 *
 * @param The static array to copy
 * @return A new static array with the specified contents
 */
template <typename _T, _T ... _V>
static inline constexpr auto make_array( static_array<_T, _V...> const & ) {
	return static_array<_T, _V...> {};
}

/*!
 * @brief Factory method to create a new compile-time static character array from an existing one
 *
 * @param The static character array to copy
 * @return A new static character array with the specified contents
 */
template <char ... _V>
static inline constexpr auto make_string( static_array<char, _V...> const &array ) {
	return static_array<char, _V...> {};
}

/*!
 * @brief Factory method to create a new compile-time static array from the given boost::hana implementation
 *
 * @param The static array to copy
 * @return A new static array with the specified contents
 */
template <typename _T, _T ... _V>
static inline constexpr auto make_array( static_array_t<_T, _V...> const & ) {
	return static_array<_T, _V...> {};
}

/*!
 * @brief Factory method to create a new compile-time static array from a slice of the given structure
 *
 * @see https://www.boost.org/doc/libs/1_62_0/libs/hana/doc/html/structboost_1_1hana_1_1tuple.html
 * @param The static array or boost::hana tuple to copy
 * @param The indexes to copy out of the tuple
 * @return A new static array with the specified contents at the specified indexes
 */
template <class _Tp, std::size_t ... _I>
static inline constexpr auto make_array(const _Tp &, const std::index_sequence<_I...> &) {
	constexpr type_unboxer<_Tp> tuple {};
	constexpr auto front = boost::hana::front(tuple);
	constexpr auto value = boost::hana::value<decltype(front)>();

	typedef std::decay_t<decltype(value)> type;

	return static_array<type, boost::hana::at(tuple, boost::hana::integral_c<std::size_t, _I>)...> {};
}

/*!
 * @brief Factory method to create a new compile-time static array from a slice of the given structure
 *
 * @see https://www.boost.org/doc/libs/1_62_0/libs/hana/doc/html/structboost_1_1hana_1_1tuple.html
 * @param The static array or boost::hana tuple to copy
 * @param The indexes to copy out of the tuple
 * @return A new static array with the specified contents at the specified indexes
 */
template <class _Tp, std::size_t ... _I>
static inline constexpr auto make_array(const _Tp &arr, std::index_sequence<_I...> &&seq) {
	return make_array(arr, seq);
}

/*!
 * @brief Factory method to create a new compile-time static array from the given boost::hana tuple
 *
 * @see https://www.boost.org/doc/libs/1_62_0/libs/hana/doc/html/structboost_1_1hana_1_1tuple.html
 * @param The boost::hana tuple to copy
 * @return A new static array with the specified contents
 */
template <typename ... _T>
static inline constexpr auto make_array( const boost::hana::tuple<_T...> &tuple ) {
	return make_array(tuple, std::make_index_sequence<sizeof...(_T)> {});
}

/*!
 * @brief Factory method to create a new compile-time static array from an integer sequence
 *
 * @param The integer sequence to use
 * @return A new static array with the specified contents
 */
template <typename _T, _T... _I>
static inline constexpr auto make_array(std::integer_sequence<_T, _I...> &&) {
	return static_array<_T, _I...> {};
}

/*!
 * @brief Factory method to create a new compile-time static array from an integer sequence
 *
 * @param The integer sequence to use
 * @return A new static array with the specified contents
 */
template <typename _T, _T... _I>
static inline constexpr auto make_array(std::integer_sequence<_T, _I...> const &) {
	return static_array<_T, _I...> {};
}

/*!
 * @brief Factory method to create a new compile-time static array from the given boost::hana string
 *
 * @see https://www.boost.org/doc/libs/1_62_0/libs/hana/doc/html/structboost_1_1hana_1_1string.html
 * @param The boost::hana string to copy
 * @return A new static character array with the specified contents
 */
template <char ... _V>
static inline constexpr auto make_array( static_string_t<_V...> const & ) {
	return static_array<char, _V...> {};
}

/*!
 * @brief Factory method to create a new compile-time static character array from the given boost::hana string
 *
 * @see https://www.boost.org/doc/libs/1_62_0/libs/hana/doc/html/structboost_1_1hana_1_1string.html
 * @param The boost::hana string to copy
 * @return A new static character array with the specified contents
 */
template <char ... _V>
static inline constexpr auto make_string( static_string_t<_V...> const & ) {
	return static_array<char, _V...> {};
}

/*!
 * @brief Structure to help builder static character strings.
 *
 * The trouble with the default builder contained in boost::hana is that it
 * assumes a constant character array and therefore the length is a sizeof()
 * operation on the array. This, of course, does not work at all for character
 * pointers, such as done with the pretty printing introspection. Therefore,
 * this class helps constructs a value character string based on a generic
 * character pointer and specified length.
 *
 * @param A class with a retrievable constant character array
 */
template <class _S>
struct static_string_builder {
	typedef _S string_type;

	static constexpr string_type str {};
	static constexpr auto value = make_string( boost::hana::string_detail::prepare_impl(str, std::make_index_sequence< str.length() > {}) );
};

/*!
 * @brief Factory method to create a new compile-time static character array using a builder class
 *
 * @return The static character array as constructed by the builder
 */
template <class _S, class _Builder = static_string_builder<_S> >
static inline constexpr auto make_string() {
	typedef _Builder string_builder;
	return string_builder::value;
}

/*!
 * @brief A class to hold a non-null constant character string
 *
 * This class is meant to be used by the static_string_builder by holding
 * a constant string reference and a known length of the string. This class
 * can be utilized by the string builder structure to make the compile-time
 * static character array.
 *
 * @see const_string
 */
template <const char * const *_Str, std::size_t _Len>
struct generic_string_holder {
	static_assert(_Str && *_Str, "Cannot use a non-null character pointer");

	static constexpr const_string value { *(_Str) };
	static constexpr decltype(auto) get() { return value.data(); }
	static constexpr auto length() { return value.length() < _Len ? value.length() : _Len; }
};

/*!
 * @brief Factory method to create a new compile-time static character array using a generic character pointer
 *
 * The string length may also be specified, but it defaults to the full string. If the length
 * specified length is less than the full character string, then the substring is returned.
 *
 * @return The static character array as constructed by the builder
 */
template <const char * const *_Str, std::size_t _Len = std::string_view::npos>
static inline constexpr auto make_string() {
	return make_string< generic_string_holder<_Str, _Len> >();
}

/*!
 * @brief A helper structure to concat static elements together and get the resulting static array
 */
template <typename _S1, typename _S2>
struct static_array_cat {
private:
    typedef typename _S1::value_type value_type;

    static_assert(std::is_same<value_type, typename _S2::value_type>::value,
                  "The two static arrays must be of the same type");

    template <value_type ... _V1, value_type ... _V2>
    static constexpr auto _concat(static_array<value_type, _V1...> const &, static_array<value_type, _V2...> const &) {
        return static_array<value_type, _V1..., _V2...> { };
    }

public:
    static constexpr auto value = _concat(_S1 { }, _S2 { });
    typedef decltype(value) type;
};

/*!
 * @brief Factory method to create a combined compile-time static array two from existing ones
 *
 * @param s1 The first static array, which determines the type of integral constants in the array
 * @param s2 The static array to concatenate at the end of the s1 array
 * @return The static array with the combined contents
 */
template <class _S1, class _S2>
static inline constexpr auto concat(_S1 const &s1, _S2 const &s2) {
	return static_array_cat<_S1, _S2>::value;
}

/*!
 * @brief Helper function to break a number into a compile-time static array of the digits
 *
 * The template parameter is broken into the digits that represent the number, given the numeric base,
 * which defaults to 10.
 *
 * @return The static array of digits
 */
template <std::size_t _Num, std::size_t _Base = 10>
static inline constexpr auto digitize() {
	constexpr auto array = static_array<uint8_t, _Num % _Base> {};
	if constexpr (_Num < _Base)
		return array;
	else
		return concat(digitize<_Num / _Base, _Base>(), array);
}

/*!
 * @brief Helper function to convert a given digit into the ASCII character equivalent
 *
 * This will work for any base up to the available number of ASCII characters, although
 * realistically it would only provide computer literate results up to a base-31,
 * or Untrigesimal, system since base-32 encodings typically use the 26 letters plus
 * digits 0-7. Realistically, most programmers only need base-2 (binary), base-8 (octal),
 * base-10 (decimal) or base-16 (hexadecimal) encodings to avoid confusion.
 *
 * @return The ASCII character for the specified digit
 */
template <std::size_t _V>
static inline constexpr auto to_char() {
	static_assert(_V < 128, "Invalid digit in string");
	return boost::hana::integral_c<char, _V < 10 ? _V + '0' : _V + ('A' - 10)>;
}

/*!
 * @brief Factory method to create a new compile-time static character array from a slice of the given structure
 *
 * It is expected that the tuple contains just a set of numeric integral constants that represents the
 * digits of a digitized number. Thus, a compile error will occur if any of the constants in the
 * tuple are outside of the appropriate ASCII character values.
 *
 * @see https://www.boost.org/doc/libs/1_62_0/libs/hana/doc/html/structboost_1_1hana_1_1tuple.html
 * @param The static array or boost::hana tuple to copy
 * @param The indexes to copy out of the tuple
 * @return A new static character array from the specified
 */
template <class _Tp, std::size_t ... _I>
static inline constexpr auto make_string(_Tp const &array, std::index_sequence<_I...> &&) {
	//static_assert(!is_static_string(array), "Parameter is not a tuple type array");
	constexpr type_unboxer<_Tp> tuple {};
	constexpr auto str = boost::hana::make_string(
			to_char<boost::hana::value(boost::hana::at(tuple, boost::hana::integral_c<std::size_t, _I>))>()...
	);

	return make_string(str);
}

/*!
 * @brief Factory method to convert the numeric value to its equivalent compile-time static character string
 *
 * @see digitize()
 * @return A new static character array for the specified number
 */
template <std::size_t _Num, std::size_t _Base = 10>
static inline constexpr auto make_string() {
	constexpr auto array = digitize<_Num, _Base>();
	return make_string(array, std::make_index_sequence<decltype(array)::size> {});
}

/*!
 * @brief Helper function to convert a compile-time static character array to its digitized equivalent array
 *
 * Each character is converted to a specific digit value without knowledge of the base. However, if the
 * character is outside the normal range of the ASCII character, then a compiler error will occur.
 *
 * @param The static character array to convert
 * @return A new static array with the specified digits
 */
template <char ... _V>
static inline constexpr auto digitize( static_array<char, _V...> const & ) {
	typedef static_array<char, _V...> type;
	constexpr auto tuple = boost::hana::tuple_c<uint8_t, _V...>;
	constexpr auto front = boost::hana::front(tuple);
	constexpr auto value = boost::hana::value<decltype(front)>();
	constexpr uint8_t digit = value <= '9' ? value - '0' : value - (value >= 'a' ? 'a' : 'A') + 10;
	static_assert(digit < 128, "Invalid digit in string");

	constexpr auto array = static_array<uint8_t, digit> {};
	if constexpr (type::size == 1)
		return array;
	else
		return concat(array, digitize(make_string(boost::hana::drop_front(typename type::type {}, boost::hana::integral_c<int, 1>))));
}

/*!
 * @brief Helper function to convert a boost::hana string to its digitized equivalent array
 *
 * Each character is converted to a specific digit value without knowledge of the base. However, if the
 * character is outside the normal range of the ASCII character, then a compiler error will occur.
 *
 * @param The boost::hana string to convert
 * @return A new static array with the specified digits
 */
template <char ... _V>
static inline constexpr auto digitize( static_string_t<_V...> const &str ) {
	return digitize(make_string(str));
}

/*!
 * @brief Helper function to convert a compile-time static structure to its number system equivalent value
 *
 * The base of the number system is assumed to be 10, but can never be detected from the values of the
 * digitized structure. A compiler error will occur if the digit falls outside of what is allowed for
 * the base. Therefore, in order to correctly expand binary, octal, or hexadecimal, for example, the
 * base MUST be explicitly specified.
 *
 * @see digitize()
 * @param The static character array or boost::hana string to convert
 * @return The numeric value represented by the digitized static array
 */
template <std::size_t _Base = 10, class _C>
static inline constexpr std::size_t dedigitize( _C const &str ) {
	constexpr type_unboxer<_C> tuple {};
	constexpr auto front = boost::hana::back(tuple);
	constexpr char digit = boost::hana::value<decltype(front)>() & 0xFF;
	static_assert(digit < _Base, "Invalid digit for base in string");

	constexpr std::size_t value = digit;
	if constexpr (_C::size == 1)
		return value;
	else
		return value + _Base * dedigitize<_Base>(make_array(boost::hana::drop_back(tuple, boost::hana::integral_c<int, 1>)));
}

/*!
 * @brief The array literal operator to quickly construct a compile-time static array from digits
 *
 * @return A new static array for the given values
 */
template <char ... _V>
constexpr auto operator"" _tmx_a() {
	return digitize( static_array<char, _V...> { } );
}

/*!
 * @brief The array literal operator to quickly construct a compile-time static array from digits
 *
 * This one works with any string literal, e.g. a hex or binary string
 *
 * @return A new static array for the given values
 */
template <typename _T, _T ... _V>
constexpr auto operator"" _tmx_a() {
	return digitize( static_array<char, _V...> { } );
}

/*!
 * @brief The string literal operator to quickly construct a compile-time static character array
 *
 *
 * @return A new static character array for the given values
 */
template <typename _T, _T ... _V>
constexpr auto operator"" _tmx_s() {
	return static_array<_T, _V ...> {};
}

/*!
 * @brief The string literal operator to quickly construct a static character array
 *
 * This one works with numeric decimal digits
 *
 * @return A new static character array for the given values
 */
template <char ... _V>
constexpr auto operator""_tmx_s() {
	return static_array<char, _V...>();
}

/*!
 * @brief Count the number of templated types contained within the array
 *
 * @return The number of template types in the object
 */
template <typename _T, _T ... _V>
struct type_count< static_array<_T, _V...> >:
		public std::integral_constant<std::size_t, sizeof...(_V)> { };

// Some useful byte sequences.

// As defined by <cctype> header.
// @see https://cplusplus.com/reference/cctype
typedef static_array<char, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09,
        0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16,
        0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 0x7F> control_characters;
typedef control_characters cntrl_characters;
typedef control_characters non_printable_characters;
typedef static_array<char, '\t', ' '> blank_characters;
typedef static_array<char, '\t', '\f', '\v', '\n', '\r', ' '> whitespace_characters;
typedef whitespace_characters space_characters;
typedef static_array<char, '0', '1', '2', '3', '4', '5', '6', '7', '8', '9'> numeric_characters;
typedef numeric_characters digit_characters;
typedef static_array<char, 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N',
		'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z'> uppercase_characters;
typedef uppercase_characters upper_characters;
typedef static_array<char, 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
		'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z'> lowercase_characters;
typedef lowercase_characters lower_characters;
typedef typename static_array_cat< uppercase_characters, lowercase_characters >::type alphabet_characters;
typedef alphabet_characters alpha_characters;
typedef static_array<char, '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D',
        'E', 'F', 'a', 'b', 'c', 'd', 'e', 'f'> hexdigit_characters;
typedef hexdigit_characters xdigit_characters;
typedef typename static_array_cat< alpha_characters, numeric_characters >::type alpha_numeric_characters;
typedef alpha_numeric_characters alnum_characters;
typedef static_array<char, '!', '"', '#', '$', '%', '&', '\'', '(', ')', '*', '+', ',',
        '-', '.', '/', ':', ';', '<', '=', '>', '?', '@', '[', '\\', ']', '^', '_', '`',
        '{', '|', '}', '~'> punctuation_characters;
typedef punctuation_characters punct_characters;
typedef typename static_array_cat< alpha_numeric_characters, punctuation_characters >::type graphical_characters;
typedef graphical_characters graph_characters;
typedef typename static_array_cat< graphical_characters, static_array<char, ' '> >::type printable_characters;
typedef typename static_array_cat< alpha_numeric_characters, static_array<char, '_', ':'> >::type valid_id_characters;
typedef typename static_array_cat< non_printable_characters, printable_characters >::type ascii_characters;

template <typename _CharT>
constexpr byte_sequence_t<_CharT> ltrim(byte_sequence_t<_CharT> const &seq,
                                        const _CharT *c = (const _CharT *)whitespace_characters::c_str()) {
    return seq.substr(seq.find_first_not_of(c));
}

template <typename _CharT>
constexpr byte_sequence_t<_CharT> rtrim(byte_sequence_t<_CharT> const &seq,
                                        const _CharT *c = (const _CharT *)whitespace_characters::c_str()) {
    return seq.substr(0, seq.find_last_not_of(c) + 1);
}

template <typename _CharT>
constexpr byte_sequence_t<_CharT> trim(byte_sequence_t<_CharT> const &seq,
                                       const _CharT *c = (const _CharT *)whitespace_characters::c_str()) {
    return ltrim(rtrim(seq, c), c);
}

} /* End namespace common */
} /* End namespace tmx */

using tmx::common::operator ""_tmx_a;
using tmx::common::operator ""_tmx_s;

/*!
 * @brief Use a constant evaluated character array variable to build a compile-time static character array
 */
#define TMX_STATIC_STRING_VAR(X) tmx::common::make_string< &X >()

/*!
 * @brief Use a constant evaluated character array variable to build a compile-time static character array
 */
#define TMX_STATIC_STRING_VAR_SUBSTR(X, N) tmx::common::make_string< &X, N >()

/*!
 * @brief Build a compile-time static string from a string literal
 */
#define TMX_STATIC_STRING(X) X ## _tmx_s

/*!
 * @brief Build a compile-time static array from a numeric literal
 */
#define TMX_STATIC_NUMBER(X) tmx::common::digitize(TMX_STATIC_STRING(X))

#define TMX_STATIC_STRING_WRAP(O, X, C) tmx::common::meta::concat( \
			tmx::common::static_array<char, O> {}, \
			tmx::common::concat(X, \
			tmx::common::static_array<char, C> {}))

#endif /* PLATFORM_INCLUDE_TMX_COMMON_PLATFORM_TYPES_ARRAYS_HPP_ */
