/*!
 * Copyright (c) 2021 Battelle Memorial Institute
 *
 * All Rights Reserved.
 *
 * @file type_traits.hpp
 *
 * This header contains functions and associated structures used to perform
 * quick, generally compile-time checks on the characteristics of a given type.
 *
 * Each function should have a signature that takes the type in question as a
 * stand-alone template parameter and one that uses a constant reference to
 * variable of that type.
 *
 * Note that the VERY helpful meta-programming library found in boost::hana
 * is used heavily in order to create some of the constant types and functions.
 *
 * @see https://www.boost.org/doc/libs/1_78_0/libs/hana/doc/html/index.html
 *
 *  Created on: Jun 15, 2021
 *      @author: Gregory M. Baumgardner
 */

#ifndef PLATFORM_INCLUDE_TMX_COMMON_PLATFORM_TYPES_TRAITS_HPP_
#define PLATFORM_INCLUDE_TMX_COMMON_PLATFORM_TYPES_TRAITS_HPP_

#include <tmx/common/platform/types/bytes.hpp>

#include <boost/hana.hpp>
#include <boost/type_traits.hpp>

#include <cstddef>
#include <type_traits>

namespace tmx {
namespace common {

// Some useful macros from the Boost::Hana documentation
#define TMX_CHECK_FOR_FUNCTION(_F) \
boost::hana::is_valid([](auto &&obj) -> decltype(obj._F()) { })
#define TMX_CHECK_FOR_TYPENAME(_T, _N) \
boost::hana::is_valid([](auto t) -> boost::hana::type<typename decltype(t)::type::_N> { }, \
boost::hana::type_c<_T>);
#define TMX_CHECK_FOR_STATIC_CONSTEXPR(_T, _N) \
boost::hana::is_valid([](auto t) -> decltype((void)decltype(t)::type::_N) { }, \
boost::hana::type_c<_T>);
#define TMX_CHAR_TUPLE_STRING(X) BOOST_HANA_STRING(X)

template <typename, typename = void>
struct _type_complete_t: public std::false_type { };

template <typename _T>
struct _type_complete_t< _T, std::void_t<decltype(sizeof(_T))> >: public std::true_type { };

template <typename _T>
using is_type_complete = typename _type_complete_t<_T>::type;

/*!
 * @brief Helper function used to test if a typedef declaration exists for the given type
 *
 * @param The type to test
 * @return True if the given type has a typedef declaration named "type"
 */
template <typename _T>
constexpr bool has_type() noexcept { return TMX_CHECK_FOR_TYPENAME(_T, type); }

/*!
 * @brief Helper function used to test if a typedef declaration exists for the given type
 *
 * @param The type to test
 * @return True if the given type has a typedef declaration named "type"
 */
template <typename _T>
constexpr bool has_type( _T const &) noexcept { return has_type<_T>(); }

/*!
 * @brief Helper structure used to un-box a type contained inside another
 *
 * If the type is a basic C++ type, then that exact type is used.
 * However, if the type is a structure with a defined typedef declaration
 * named "type", then that boxed type is used.
 */
template <typename _T, bool>
struct _type_unboxer { typedef _T type; };

template <typename _T>
struct _type_unboxer<_T, true> { typedef typename _T::type type; };

/*!
 * @brief An alias to un-box a type contained inside another
 *
 * @see _type_unboxer
 */
template <typename _T>
using type_unboxer = typename _type_unboxer<_T, has_type<_T>()>::type;

/*!
 * @brief Helper function used to test if a typedef declaration exists for the given type
 *
 * @param The type to test
 * @return True if the given type has a typedef declaration named "type"
 */
template <typename _T>
constexpr bool has_value_type() noexcept { return TMX_CHECK_FOR_TYPENAME(_T, value_type); }

/*!
 * @brief Helper function used to test if a typedef declaration exists for the given type
 *
 * @param The type to test
 * @return True if the given type has a typedef declaration named "type"
 */
template <typename _T>
constexpr bool has_value_type( _T const &) noexcept { return has_value_type<_T>(); }

/*!
 * @brief Helper structure used to un-box a type contained inside another
 *
 * If the type is a basic C++ type, then that exact type is used.
 * However, if the type is a structure with a defined typedef declaration
 * named "value_type", then that boxed type is used.
 */
template <typename _T, bool>
struct _value_type_unboxer { typedef _T type; };

template <typename _T>
struct _value_type_unboxer<_T, true> { typedef typename _T::value_type type; };

/*!
 * @brief An alias to un-box a type contained inside another
 *
 * @see _type_unboxer
 */
template <typename _T>
using value_type_unboxer = typename _value_type_unboxer<_T, has_value_type<_T>()>::type;


/*!
 * @brief Helper function used to test if a static constant expression "name" exists for the given type
 *
 * @param The type to test
 * @return True if the given type has a static constant expression named "name"
 */
template <typename _T>
constexpr bool has_name() noexcept { return TMX_CHECK_FOR_STATIC_CONSTEXPR(_T, name); }

/*!
 * @brief Helper function used to test if a static constant expression "name" exists for the given type
 *
 * @param The type to test
 * @return True if the given type has a static constant expression named "name"
 */
template <typename _T>
constexpr bool has_name(_T const &) noexcept { return has_name<_T>(); }

/*!
 * @brief Helper function used to test if a static constant expression "size" exists for the given type
 *
 * @param The type to test
 * @return True if the given type has a static constant expression named "size"
 */
template <typename _T>
constexpr bool has_size() noexcept { return TMX_CHECK_FOR_STATIC_CONSTEXPR(_T, size); }

/*!
 * @brief Helper function used to test if a static constant expression "value" exists for the given type
 *
 * @param The type to test
 * @return True if the given type has a static constant expression named "value"
 */
template <typename _T>
constexpr bool has_value() noexcept { return TMX_CHECK_FOR_STATIC_CONSTEXPR(_T, value); }

/*!
 * @brief Helper function used to test if a static constant expression "value" exists for the given type
 *
 * @param The type to test
 * @return True if the given type has a static constant expression named "value"
 */
template <typename _T>
constexpr bool has_value(_T const &) noexcept { return has_value<_T>(); }

/*!
 * @brief Helper function used to test if a static constant expression "size" exists for the given type
 *
 * @param The type to test
 * @return True if the given type has a static constant expression named "size"
 */
template <typename _T>
constexpr bool has_size(_T const &) noexcept { return has_size<_T>(); }

template <typename _T, typename _Other, typename _Tp = _T>
using enable_type = std::enable_if_t<std::is_same_v<_T, _Other>, _Tp>;

template <typename _T, typename _Other, typename _Tp = _T>
using disable_type = std::enable_if<!std::is_same_v<_T, _Other>, _Tp>;

/*!
 * @see #std::is_same
 * @return true_type if _T is a boolean type. false_type otherwise.
 */
template <typename _T>
using is_bool = std::integral_constant<bool, std::is_same_v<bool, std::decay_t<_T> > >;

/*!
 * @return True if _T is an integral type. False otherwise.
 */
template <typename _T>
constexpr bool is_boolean_type() noexcept {
	return is_bool<_T>::value;
}

/*!
 * @return _Tp if _T is a boolean. Undefined otherwise
 */
template <typename _T, typename _Tp = _T>
using enable_bool =
		typename std::enable_if<is_boolean_type<_T>(), _Tp>::type;

/*!
 * @return _Tp if _T is a not boolean. Undefined otherwise
 */
template <typename _T, typename _Tp = _T>
using disable_bool =
		typename std::enable_if<!is_boolean_type<_T>(), _Tp>::type;

/*!
 * @see #std::is_integral
 * @return true_type if _T is an integral type. false_type otherwise.
 */
template <typename _T>
using is_integer = std::is_integral<std::decay_t<_T> >;

/*!
 * @return True if _T is an integral type. False otherwise.
 */
template <typename _T>
constexpr bool is_integer_type() noexcept {
	return is_integer<_T>::value;
}

/*!
 * @return True if the argument is an integral type. False otherwise.
 */
template <typename _T>
constexpr bool is_integer_type(_T const &) noexcept {
	return is_integer_type<_T>();
}

/*!
 * @return _Tp if _T is an integer. Undefined otherwise
 */
template <typename _T, typename _Tp = _T>
using enable_int =
		typename std::enable_if<is_integer_type<_T>(), _Tp>::type;

/*!
 * @return _Tp if _T is not an integer. Undefined otherwise
 */
template <typename _T, typename _Tp = _T>
using disable_int =
		typename std::enable_if<!is_integer_type<_T>(), _Tp>::type;

/*!
 * @see #std::is_signed
 * @return true_type if _T is a signed integer. false_type otherwise.
 */
template <typename _T>
using is_signed_int = std::integral_constant<bool,
		is_integer_type<_T>() && std::is_signed_v<std::decay_t<_T> > >;

/*!
 * @return True if _T is a signed integer. False otherwise.
 */
template <typename _T>
constexpr bool is_signed_integer() noexcept {
	return is_signed_int<_T>::value;
}

/*!
 * @return True if the argument is a signed integer. False otherwise.
 */
template <typename _T>
constexpr bool is_signed_integer(_T const &) noexcept {
	return is_signed_integer<_T>();
}

/*!
 * @return _Tp if _T is a signed integer. Undefined otherwise
 */
template <typename _T, typename _Tp = _T>
using enable_signed_int =
		typename std::enable_if<is_signed_integer<_T>(), _Tp>::type;

/*!
 * @return _Tp if _T is not a signed integer. Undefined otherwise
 */
template <typename _T, typename _Tp = _T>
using disable_signed_int =
		typename std::enable_if<!is_signed_integer<_T>(), _Tp>::type;

/*!
 * @see #std::is_signed
 * @return true_type if _T is an unsigned integer. false_type otherwise.
 */
template <typename _T>
using is_unsigned_int = std::integral_constant<bool,
		is_integer_type<_T>() && !std::is_signed_v<std::decay_t<_T> > >;

/*!
 * @return True if _T is an unsigned integer. False otherwise.
 */
template <typename _T>
constexpr bool is_unsigned_integer() noexcept {
	return is_unsigned_int<_T>::value;
}

/*!
 * @return True if _T is an unsigned integer. False otherwise.
 */
template <typename _T>
constexpr bool is_unsigned_integer(_T const &) noexcept {
	return is_unsigned_integer<_T>();
}

/*!
 * @return _Tp if _T is an unsigned integer. Undefined otherwise
 */
template <typename _T, typename _Tp = _T>
using enable_unsigned_int =
		typename std::enable_if<is_unsigned_integer<_T>(), _Tp>::type;

/*!
 * @return _Tp if _T is not an unsigned integer. Undefined otherwise
 */
template <typename _T, typename _Tp = _T>
using disable_unsigned_int =
		typename std::enable_if<!is_unsigned_integer<_T>(), _Tp>::type;

/*!
 * @see #std::is_floating_point
 * @return true_type if _T is a floating point type. false_type otherwise.
 */
template <typename _T>
using is_float = std::is_floating_point<std::decay_t<_T> >;

/*!
 * @return True if _T is a floating point type. False otherwise.
 */
template <typename _T>
constexpr bool is_float_type() noexcept {
	return is_float<_T>::value;
}

/*!
 * @return True if the argument is a floating point type. False otherwise.
 */
template <typename _T>
constexpr bool is_float_type(_T const &) noexcept {
	return is_float_type<_T>();
}

/*!
 * @return _Tp if _T is a floating point type. Undefined otherwise
 */
template <typename _T, typename _Tp = _T>
using enable_float =
		typename std::enable_if<is_float_type<_T>(), _Tp>::type;

/*!
 * @return _Tp if _T is not a floating point type. Undefined otherwise
 */
template <typename _T, typename _Tp = _T>
using disable_float =
		typename std::enable_if<!is_float_type<_T>(), _Tp>::type;

/*!
 * @see #std::is_arithmetic
 * @return true_type if _T is an arithmetic type. false_type otherwise.
 */
template <typename _T>
using is_arithmetic = std::integral_constant<bool, std::is_arithmetic_v<std::decay_t<_T> > >;

/*!
 * @return True if _T is an arithmetic type. False otherwise.
 */
template <typename _T>
constexpr bool is_arithmetic_type() noexcept {
	return is_arithmetic<_T>::value;
}

/*!
 * @return True if _T is an arithmetic type. False otherwise.
 */
template <typename _T>
constexpr bool is_arithmetic_type(_T const &) noexcept {
	return is_arithmetic_type<_T>();
}

/*!
 * @return _Tp if _T is an arithmetic type. Undefined otherwise
 */
template <typename _T, typename _Tp = _T>
using enable_arithmetic =
		typename std::enable_if<is_arithmetic_type<_T>(), _Tp>::type;

/*!
 * @return _Tp if _T is not an arithmetic type. Undefined otherwise
 */
template <typename _T, typename _Tp = _T>
using disable_arithmetic =
		typename std::enable_if<!is_arithmetic_type<_T>(), _Tp>::type;

/*!
 * @see #std::is_scalar
 * @return true_type if _T is a scalar type. false_type otherwise.
 */
template <typename _T>
using is_scalar = std::integral_constant<bool, std::is_scalar_v<std::decay_t<_T> > >;

/*!
 * @return True if _T is a scalar type. False otherwise.
 */
template <typename _T>
constexpr bool is_scalar_type() noexcept {
	return is_scalar<_T>::value;
}

/*!
 * @return True if _T is a scalar type. False otherwise.
 */
template <typename _T>
constexpr bool is_scalar_type(_T const &) noexcept {
	return is_scalar_type<_T>();
}

/*!
 * @return _Tp if _T is a scalar type. Undefined otherwise
 */
template <typename _T, typename _Tp = _T>
using enable_scalar =
		typename std::enable_if<is_scalar_type<_T>(), _Tp>::type;

/*!
 * @return _Tp if _T is not a scalar type. Undefined otherwise
 */
template <typename _T, typename _Tp = _T>
using disable_scalar =
		typename std::enable_if<!is_scalar_type<_T>(), _Tp>::type;

/*!
 * @see #std::is_enum
 * @return true_type if _T is a enumeration type. false_type otherwise.
 */
template <typename _T>
using is_enum = std::integral_constant<bool, std::is_enum_v<std::decay_t<_T> > >;

/*!
 * @return True if _T is a C++ enum type. False otherwise.
 */
template <typename _T>
constexpr bool is_enum_type() noexcept {
	return is_enum<_T>::value;
}

/*!
 * @return True if _T is a C++ enum type. False otherwise.
 */
template <typename _T>
constexpr bool is_enum_type(_T const &) noexcept {
	return is_enum_type<_T>();
}

/*!
 * @return _Tp if _T is an enum type. Undefined otherwise
 */
template <typename _T, typename _Tp = _T>
using enable_enum =
		typename std::enable_if<is_enum_type<_T>(), _Tp>::type;

/*!
 * @return _Tp if _T is not an enum type. Undefined otherwise
 */
template <typename _T, typename _Tp = _T>
using disable_enum =
		typename std::enable_if<!is_enum_type<_T>(), _Tp>::type;

/*!
 * @see #std::is_convertible
 * @return true_type if _T is a string type. false_type otherwise.
 */
template <typename _T, typename _CharT = char_t>
using is_string = std::integral_constant<bool, std::is_convertible_v<std::decay_t<_T>, char_sequence<_CharT> > >;

/*!
 * @return True if _T is a C or C++ character sequence type. False otherwise.
 */
template <typename _T, typename _CharT = char_t>
constexpr bool is_string_type() noexcept {
	return is_string<_T, _CharT>::value;
}

/*!
 * @return True if _T is a C or C++ character sequence type. False otherwise.
 */
template <typename _T, typename _CharT = char_t>
constexpr bool is_string_type(_T const &) noexcept {
	return is_string_type<_T, _CharT>();
}

/*!
 * @return _Tp if _T is a C or C++ character sequence type. Undefined otherwise
 */
template <typename _T, typename _Tp = _T, typename _CharT = char_t>
using enable_string =
		typename std::enable_if<is_string_type<_T, _CharT>(), _Tp>::type;

/*!
 * @return _Tp if _T is not a C or C++ character sequence type. Undefined otherwise
 */
template <typename _T, typename _Tp = _T, typename _CharT = char_t>
using disable_string =
		typename std::enable_if<!is_string_type<_T, _CharT>(), _Tp>::type;

/*!
 * @return true_type if _T == _Other is valid, false_type otherwise
 */
template <class _T, class _Other = _T>
using has_equal = boost::has_equal_to<_T, _Other>;

/*!
 * @return True if _T == _Other is valid. False otherwise
 */
template <class _T, class _Other = _T>
constexpr bool has_operator_equal() noexcept {
	return has_equal<_T, _Other>::value;
}

/*!
 * @return True if _T == _Other is valid. False otherwise
 */
template <class _T, class _Other = _T>
constexpr bool has_operator_equal(_T const &) noexcept {
	return has_operator_equal<_T, _Other>();
}

/*!
 * @return _Tp if _T == _Other is valid. Undefined otherwise
 */
template <class _T, class _Other = _T, class _Tp = _T>
using enable_equal =
		typename std::enable_if<has_operator_equal<_T, _Other>(), _Tp>::type;

/*!
 * @return _Tp if _T == _Other is not valid. Undefined otherwise
 */
template <class _T, class _Other = _T, class _Tp = _T>
using disable_equal =
		typename std::enable_if<!has_operator_equal<_T, _Other>(), _Tp>::type;

/*!
 * @return true_type if _T != _Other is valid, false_type otherwise
 */
template <class _T, class _Other = _T>
using has_not_equal = boost::has_not_equal_to<_T, _Other>;

/*!
 * @return True if _T != _Other is valid. False otherwise
 */
template <class _T, class _Other = _T>
constexpr bool has_operator_not_equal() noexcept {
	return has_not_equal<_T, _Other>::value;
}

/*!
 * @return True if _T != _Other is valid. False otherwise
 */
template <class _T, class _Other = _T>
constexpr bool has_operator_not_equal(_T const &) noexcept {
	return has_operator_not_equal<_T, _Other>();
}

/*!
 * @return _Tp if _T != _Other is valid. Undefined otherwise
 */
template <class _T, class _Other = _T, class _Tp = _T>
using enable_not_equal =
		typename std::enable_if<has_operator_not_equal<_T, _Other>(), _Tp>::type;

/*!
 * @return _Tp if _T != _Other is not valid. Undefined otherwise
 */
template <class _T, class _Other = _T, class _Tp = _T>
using disable_not_equal =
		typename std::enable_if<!has_operator_not_equal<_T, _Other>(), _Tp>::type;

/*!
 * @return true_type if _T < _Other is valid, false_type otherwise
 */
template <class _T, class _Other = _T>
using has_less = boost::has_less<_T, _Other>;

/*!
 * @return True if _T < _Other is valid. False otherwise
 */
template <class _T, class _Other = _T>
constexpr bool has_operator_less() noexcept {
	return has_less<_T, _Other>::value;
}

/*!
 * @return True if _T < _Other is valid. False otherwise
 */
template <class _T, class _Other = _T>
constexpr bool has_operator_less(_T const &) noexcept {
	return has_operator_less<_T, _Other>();
}

/*!
 * @return _Tp if _T < _Other is valid. Undefined otherwise
 */
template <class _T, class _Other = _T, class _Tp = _T>
using enable_less =
		typename std::enable_if<has_operator_less<_T, _Other>(), _Tp>::type;

/*!
 * @return _Tp if _T < _Other is not valid. Undefined otherwise
 */
template <class _T, class _Other = _T, class _Tp = _T>
using disable_less =
		typename std::enable_if<!has_operator_less<_T, _Other>(), _Tp>::type;

/*!
 * @return true_type if _T > _Other is valid, false_type otherwise
 */
template <class _T, class _Other = _T>
using has_greater = boost::has_greater<_T, _Other>;

/*!
 * @return True if _T > _Other is valid. False otherwise
 */
template <class _T, class _Other = _T>
constexpr bool has_operator_greater() noexcept {
	return has_greater<_T, _Other>::value;
}

/*!
 * @return True if _T > _Other is valid. False otherwise
 */
template <class _T, class _Other = _T>
constexpr bool has_operator_greater(_T const &) noexcept {
	return has_operator_greater<_T, _Other>();
}

/*!
 * @return _Tp if _T > _Other is valid. Undefined otherwise
 */
template <class _T, class _Other = _T, class _Tp = _T>
using enable_greater =
		typename std::enable_if<has_operator_greater<_T, _Other>(), _Tp>::type;

/*!
 * @return _Tp if _T > _Other is not valid. Undefined otherwise
 */
template <class _T, class _Other = _T, class _Tp = _T>
using disable_greater =
		typename std::enable_if<!has_operator_greater<_T, _Other>(), _Tp>::type;

/*!
 * @return true_type if _T <= _Other is valid, false_type otherwise
 */
template <class _T, class _Other = _T>
using has_less_equal = boost::has_less_equal<_T, _Other>;

/*!
 * @return True if _T <= _Other is valid. False otherwise
 */
template <class _T, class _Other = _T>
constexpr bool has_operator_less_equal() noexcept {
	return has_less_equal<_T, _Other>::value;
}

/*!
 * @return True if _T <= _Other is valid. False otherwise
 */
template <class _T, class _Other = _T>
constexpr bool has_operator_less_equal(_T const &) noexcept {
	return has_operator_less_equal<_T, _Other>();
}

/*!
 * @return _Tp if _T <= _Other is valid. Undefined otherwise
 */
template <class _T, class _Other = _T, class _Tp = _T>
using enable_less_equal =
		typename std::enable_if<has_operator_less_equal<_T, _Other>(), _Tp>::type;

/*!
 * @return _Tp if _T <= _Other is not valid. Undefined otherwise
 */
template <class _T, class _Other = _T, class _Tp = _T>
using disable_less_equal =
		typename std::enable_if<!has_operator_less_equal<_T, _Other>(), _Tp>::type;

/*!
 * @return true_type if _T >= _Other is valid, false_type otherwise
 */
template <class _T, class _Other = _T>
using has_greater_equal = boost::has_greater_equal<_T, _Other>;

/*!
 * @return True if _T >= _Other is valid. False otherwise
 */
template <class _T, class _Other = _T>
constexpr bool has_operator_greater_equal() noexcept {
	return has_greater_equal<_T, _Other>::value;
}

/*!
 * @return True if _T >= _Other is valid. False otherwise
 */
template <class _T, class _Other = _T>
constexpr bool has_operator_greater_equal(_T const &) noexcept {
	return has_operator_greater_equal<_T, _Other>();
}

/*!
 * @return _Tp if _T >= _Other is valid. Undefined otherwise
 */
template <class _T, class _Other = _T, class _Tp = _T>
using enable_greater_equal =
		typename std::enable_if<has_operator_greater_equal<_T, _Other>(), _Tp>::type;

/*!
 * @return _Tp if _T >= _Other is not valid. Undefined otherwise
 */
template <class _T, class _Other = _T, class _Tp = _T>
using disable_greater_equal =
		typename std::enable_if<!has_operator_greater_equal<_T, _Other>(), _Tp>::type;

/*!
 * @return true_type if +_T is valid, false_type otherwise
 */
template <class _T>
using has_unary_plus = boost::has_unary_plus<_T>;

/*!
 * @return True if +_T is valid. False otherwise
 */
template <class _T>
constexpr bool has_operator_unary_plus() noexcept {
	return has_unary_plus<_T>::value;
}

/*!
 * @return True if +_T is valid. False otherwise
 */
template <class _T>
constexpr bool has_operator_unary_plus(_T const &) noexcept {
	return has_operator_unary_plus<_T>();
}

/*!
 * @return _Tp if +_T is valid. Undefined otherwise
 */
template <class _T, class _Tp = _T>
using enable_unary_plus =
		typename std::enable_if<has_operator_unary_plus<_T>(), _Tp>::type;

/*!
 * @return _Tp if +_T is not valid. Undefined otherwise
 */
template <class _T, class _Tp = _T>
using disable_unary_plus =
		typename std::enable_if<!has_operator_unary_plus<_T>(), _Tp>::type;

/*!
 * @return true_type if -_T is valid, false_type otherwise
 */
template <class _T, class _Other = _T>
using has_unary_minus = boost::has_unary_minus<_T, _Other>;

/*!
 * @return True if -_T  is valid. False otherwise
 */
template <class _T>
constexpr bool has_operator_unary_minus() noexcept {
	return has_unary_minus<_T>::value;
}

/*!
 * @return True if -_T is valid. False otherwise
 */
template <class _T>
constexpr bool has_operator_unary_minus(_T const &) noexcept {
	return has_operator_unary_minus<_T>();
}

/*!
 * @return _Tp if -_T is valid. Undefined otherwise
 */
template <class _T, class _Tp = _T>
using enable_unary_minus =
		typename std::enable_if<has_operator_unary_minus<_T>(), _Tp>::type;

/*!
 * @return _Tp if -_T is not valid. Undefined otherwise
 */
template <class _T, class _Tp = _T>
using disable_unary_minus =
		typename std::enable_if<!has_operator_unary_minus<_T>(), _Tp>::type;

/*!
 * @return true_type if _T + _Other is valid, false_type otherwise
 */
template <class _T, class _Other = _T>
using has_plus = boost::has_plus<_T, _Other>;

/*!
 * @return True if _T + _Other is valid. False otherwise
 */
template <class _T, class _Other = _T>
constexpr bool has_operator_plus() noexcept {
	return has_plus<_T, _Other>::value;
}

/*!
 * @return True if _T + _Other is valid. False otherwise
 */
template <class _T, class _Other = _T>
constexpr bool has_operator_plus(_T const &) noexcept {
	return has_operator_plus<_T, _Other>();
}

/*!
 * @return _Tp if _T + _Other is valid. Undefined otherwise
 */
template <class _T, class _Other = _T, class _Tp = _T>
using enable_plus =
		typename std::enable_if<has_operator_plus<_T, _Other>(), _Tp>::type;

/*!
 * @return _Tp if _T + _Other is not valid. Undefined otherwise
 */
template <class _T, class _Other = _T, class _Tp = _T>
using disable_plus =
		typename std::enable_if<!has_operator_plus<_T, _Other>(), _Tp>::type;

/*!
 * @return true_type if _T - _Other is valid, false_type otherwise
 */
template <class _T, class _Other = _T>
using has_minus = boost::has_minus<_T, _Other>;

/*!
 * @return True if _T - _Other is valid. False otherwise
 */
template <class _T, class _Other = _T>
constexpr bool has_operator_minus() noexcept {
	return has_minus<_T, _Other>::value;
}

/*!
 * @return True if _T - _Other is valid. False otherwise
 */
template <class _T, class _Other = _T>
constexpr bool has_operator_minus(_T const &) noexcept {
	return has_operator_minus<_T, _Other>();
}

/*!
 * @return _Tp if _T - _Other is valid. Undefined otherwise
 */
template <class _T, class _Other = _T, class _Tp = _T>
using enable_minus =
		typename std::enable_if<has_operator_minus<_T, _Other>(), _Tp>::type;

/*!
 * @return _Tp if _T - _Other is not valid. Undefined otherwise
 */
template <class _T, class _Other = _T, class _Tp = _T>
using disable_minus =
		typename std::enable_if<!has_operator_minus<_T, _Other>(), _Tp>::type;

/*!
 * @return true_type if _T * _Other is valid, false_type otherwise
 */
template <class _T, class _Other = _T>
using has_multiplies = boost::has_multiplies<_T, _Other>;

/*!
 * @return True if _T * _Other is valid. False otherwise
 */
template <class _T, class _Other = _T>
constexpr bool has_operator_multiplies() noexcept {
	return has_multiplies<_T, _Other>::value;
}

/*!
 * @return True if _T * _Other is valid. False otherwise
 */
template <class _T, class _Other = _T>
constexpr bool has_operator_multiplies(_T const &) noexcept {
	return has_operator_multiplies<_T, _Other>();
}

/*!
 * @return _Tp if _T * _Other is valid. Undefined otherwise
 */
template <class _T, class _Other = _T, class _Tp = _T>
using enable_multiplies =
		typename std::enable_if<has_operator_multiplies<_T, _Other>(), _Tp>::type;

/*!
 * @return _Tp if _T * _Other is not valid. Undefined otherwise
 */
template <class _T, class _Other = _T, class _Tp = _T>
using disable_multiplies =
		typename std::enable_if<!has_operator_multiplies<_T, _Other>(), _Tp>::type;

/*!
 * @return true_type if _T / _Other is valid, false_type otherwise
 */
template <class _T, class _Other = _T>
using has_divides = boost::has_divides<_T, _Other>;

/*!
 * @return True if _T / _Other is valid. False otherwise
 */
template <class _T, class _Other = _T>
constexpr bool has_operator_divides() noexcept {
	return has_divides<_T, _Other>::value;
}

/*!
 * @return True if _T / _Other is valid. False otherwise
 */
template <class _T, class _Other = _T>
constexpr bool has_operator_divides(_T const &) noexcept {
	return has_operator_divides<_T, _Other>();
}

/*!
 * @return _Tp if _T / _Other is valid. Undefined otherwise
 */
template <class _T, class _Other = _T, class _Tp = _T>
using enable_divides =
		typename std::enable_if<has_operator_divides<_T, _Other>(), _Tp>::type;

/*!
 * @return _Tp if _T / _Other is not valid. Undefined otherwise
 */
template <class _T, class _Other = _T, class _Tp = _T>
using disable_divides =
		typename std::enable_if<!has_operator_divides<_T, _Other>(), _Tp>::type;

/*!
 * @return true_type if _T % _Other is valid, false_type otherwise
 */
template <class _T, class _Other = _T>
using has_modulus = boost::has_modulus<_T, _Other>;

/*!
 * @return True if _T % _Other is valid. False otherwise
 */
template <class _T, class _Other = _T>
constexpr bool has_operator_modulus() noexcept {
	return has_modulus<_T, _Other>::value;
}

/*!
 * @return True if _T % _Other is valid. False otherwise
 */
template <class _T, class _Other = _T>
constexpr bool has_operator_modulus(_T const &) noexcept {
	return has_operator_modulus<_T, _Other>();
}

/*!
 * @return _Tp if _T % _Other is valid. Undefined otherwise
 */
template <class _T, class _Other = _T, class _Tp = _T>
using enable_modulus =
		typename std::enable_if<has_operator_modulus<_T, _Other>(), _Tp>::type;

/*!
 * @return _Tp if _T % _Other is not valid. Undefined otherwise
 */
template <class _T, class _Other = _T, class _Tp = _T>
using disable_modulus =
		typename std::enable_if<!has_operator_modulus<_T, _Other>(), _Tp>::type;

/*!
 * @return true_type if _T += _Other is valid, false_type otherwise
 */
template <class _T, class _Other = _T>
using has_plus_assign = boost::has_plus_assign<_T, _Other>;

/*!
 * @return True if _T += _Other is valid. False otherwise
 */
template <class _T, class _Other = _T>
constexpr bool has_operator_plus_assign() noexcept {
	return has_plus_assign<_T, _Other>::value;
}

/*!
 * @return True if _T += _Other is valid. False otherwise
 */
template <class _T, class _Other = _T>
constexpr bool has_operator_plus_assign(_T const &) noexcept {
	return has_operator_plus_assign<_T, _Other>();
}

/*!
 * @return _Tp if _T += _Other is valid. Undefined otherwise
 */
template <class _T, class _Other = _T, class _Tp = _T>
using enable_plus_assign =
		typename std::enable_if<has_operator_plus_assign<_T, _Other>(), _Tp>::type;

/*!
 * @return _Tp if _T += _Other is not valid. Undefined otherwise
 */
template <class _T, class _Other = _T, class _Tp = _T>
using disable_plus_assign =
		typename std::enable_if<!has_operator_plus_assign<_T, _Other>(), _Tp>::type;

/*!
 * @return true_type if _T -= _Other is valid, false_type otherwise
 */
template <class _T, class _Other = _T>
using has_minus_assign = boost::has_minus_assign<_T, _Other>;

/*!
 * @return True if _T -= _Other is valid. False otherwise
 */
template <class _T, class _Other = _T>
constexpr bool has_operator_minus_assign() noexcept {
	return has_minus_assign<_T, _Other>::value;
}

/*!
 * @return True if _T -= _Other is valid. False otherwise
 */
template <class _T, class _Other = _T>
constexpr bool has_operator_minus_assign(_T const &) noexcept {
	return has_operator_minus_assign<_T, _Other>();
}

/*!
 * @return _Tp if _T -= _Other is valid. Undefined otherwise
 */
template <class _T, class _Other = _T, class _Tp = _T>
using enable_minus_assign =
		typename std::enable_if<has_operator_minus_assign<_T, _Other>(), _Tp>::type;

/*!
 * @return _Tp if _T -= _Other is not valid. Undefined otherwise
 */
template <class _T, class _Other = _T, class _Tp = _T>
using disable_minus_assign =
		typename std::enable_if<!has_operator_minus_assign<_T, _Other>(), _Tp>::type;

/*!
 * @return true_type if _T *= _Other is valid, false_type otherwise
 */
template <class _T, class _Other = _T>
using has_multiplies_assign = boost::has_multiplies_assign<_T, _Other>;

/*!
 * @return True if _T *= _Other is valid. False otherwise
 */
template <class _T, class _Other = _T>
constexpr bool has_operator_multiplies_assign() noexcept {
	return has_multiplies_assign<_T, _Other>::value;
}

/*!
 * @return True if _T *= _Other is valid. False otherwise
 */
template <class _T, class _Other = _T>
constexpr bool has_operator_multiplies_assign(_T const &) noexcept {
	return has_operator_multiplies_assign<_T, _Other>();
}

/*!
 * @return _Tp if _T *= _Other is valid. Undefined otherwise
 */
template <class _T, class _Other = _T, class _Tp = _T>
using enable_multiplies_assign =
		typename std::enable_if<has_operator_multiplies_assign<_T, _Other>(), _Tp>::type;

/*!
 * @return _Tp if _T *= _Other is not valid. Undefined otherwise
 */
template <class _T, class _Other = _T, class _Tp = _T>
using disable_multiplies_assign =
		typename std::enable_if<!has_operator_multiplies_assign<_T, _Other>(), _Tp>::type;

/*!
 * @return true_type if _T /= _Other is valid, false_type otherwise
 */
template <class _T, class _Other = _T>
using has_divides_assign = boost::has_divides_assign<_T, _Other>;

/*!
 * @return True if _T /= _Other is valid. False otherwise
 */
template <class _T, class _Other = _T>
constexpr bool has_operator_divides_assign() noexcept {
	return has_divides_assign<_T, _Other>::value;
}

/*!
 * @return True if _T /= _Other is valid. False otherwise
 */
template <class _T, class _Other = _T>
constexpr bool has_operator_divides_assign(_T const &) noexcept {
	return has_operator_divides_assign<_T, _Other>();
}

/*!
 * @return _Tp if _T /= _Other is valid. Undefined otherwise
 */
template <class _T, class _Other = _T, class _Tp = _T>
using enable_divides_assign =
		typename std::enable_if<has_operator_divides_assign<_T, _Other>(), _Tp>::type;

/*!
 * @return _Tp if _T /= _Other is not valid. Undefined otherwise
 */
template <class _T, class _Other = _T, class _Tp = _T>
using disable_divides_assign =
		typename std::enable_if<!has_operator_divides_assign<_T, _Other>(), _Tp>::type;

/*!
 * @return true_type if _T %= _Other is valid, false_type otherwise
 */
template <class _T, class _Other = _T>
using has_modulus_assign = boost::has_modulus_assign<_T, _Other>;

/*!
 * @return True if _T %= _Other is valid. False otherwise
 */
template <class _T, class _Other = _T>
constexpr bool has_operator_modulus_assign() noexcept {
	return has_modulus_assign<_T, _Other>::value;
}

/*!
 * @return True if _T %= _Other is valid. False otherwise
 */
template <class _T, class _Other = _T>
constexpr bool has_operator_modulus_assign(_T const &) noexcept {
	return has_operator_modulus_assign<_T, _Other>();
}

/*!
 * @return _Tp if _T %= _Other is valid. Undefined otherwise
 */
template <class _T, class _Other = _T, class _Tp = _T>
using enable_modulus_assign =
		typename std::enable_if<has_operator_modulus_assign<_T, _Other>(), _Tp>::type;

/*!
 * @return _Tp if _T %= _Other is not valid. Undefined otherwise
 */
template <class _T, class _Other = _T, class _Tp = _T>
using disable_modulus_assign =
		typename std::enable_if<!has_operator_modulus_assign<_T, _Other>(), _Tp>::type;

/*!
 * @brief Determine if _T supports operator&
 */
template <class _T, class _Other = _T>
using has_op_bit_and = boost::has_bit_and<_T, _Other>;

/*!
 * @brief Determine if _T supports operator|
 */
template <class _T, class _Other = _T>
using has_op_bit_or = boost::has_bit_or<_T, _Other>;

/*!
 * @brief Determine if _T supports operator^
 */
template <class _T, class _Other = _T>
using has_op_bit_xor = boost::has_bit_xor<_T, _Other>;

/*!
 * @brief Determine if _T supports operator&=
 */
template <class _T, class _Other = _T>
using has_op_bit_and_assign = boost::has_bit_and_assign<_T, _Other>;

/*!
 * @brief Determine if _T supports operator|=
 */
template <class _T, class _Other = _T>
using has_op_bit_or_assign = boost::has_bit_or_assign<_T, _Other>;

/*!
 * @brief Determine if _T supports operator^=
 */
template <class _T, class _Other = _T>
using has_op_bit_xor_assign = boost::has_bit_xor_assign<_T, _Other>;

/*!
 * @brief Count the number of templated types contained within the class
 *
 * @return The number of template types in the object, which is 1 for a non-template
 */
template <typename>
struct type_count:
		public std::integral_constant<std::size_t, 1> { };

/*!
 * @brief Count the number of templated types contained within the class
 *
 * @return The number of template types in the object
 */
template <template <typename ...> class _C, typename ... _T>
struct type_count< _C<_T...> >:
		public std::integral_constant<std::size_t, sizeof...(_T)> { };

/*!
 * @brief Count the number of templated types contained within the class
 *
 * Note that an argument is needed for this to be interpreted correctly.
 *
 * @param A reference to the object
 * @return The number of template types in the object
 */
template <class _C>
constexpr std::size_t type_count_of(_C const &) noexcept {
	return type_count<_C>::value;
}

template <typename _T, _T _I, _T _J>
using maximum_of = std::conditional_t<(_I >= _J), std::integral_constant<_T, _I>, std::integral_constant<_T, _J> >;

template <typename _T, _T _I, _T _J>
using minimum_of = std::conditional_t<(_I <= _J), std::integral_constant<_T, _I>, std::integral_constant<_T, _J> >;

} /* End namespace common */
} /* End namespace tmx */

#endif /* PLATFORM_INCLUDE_TMX_COMMON_PLATFORM_TYPES_TRAITS_HPP_ */
