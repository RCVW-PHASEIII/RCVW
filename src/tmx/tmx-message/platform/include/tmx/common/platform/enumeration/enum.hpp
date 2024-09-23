/*
 * Copyright (c) 2022 Battelle Memorial Institute
 *
 * All Rights Reserved.
 *
 * @file enum.hpp
 *
 *  Created on: Oct 5, 2022
 *      @author: gmb
 */

#ifndef PLATFORM_INCLUDE_TMX_COMMON_PLATFORM_ENUMERATION_ENUM_HPP_
#define PLATFORM_INCLUDE_TMX_COMMON_PLATFORM_ENUMERATION_ENUM_HPP_

// Allow for bigger enum values in TMX, if necessary
#ifndef TMX_ENUM_RANGE_MIN
#define TMX_ENUM_RANGE_MIN -500
#endif
#ifndef TMX_ENUM_RANGE_MAX
#define TMX_ENUM_RANGE_MAX  1500
#endif

#define MAGIC_ENUM_RANGE_MIN TMX_ENUM_RANGE_MIN
#define MAGIC_ENUM_RANGE_MAX TMX_ENUM_RANGE_MAX

#ifndef TMX_ENUM_CASE_COMPARE
#define TMX_ENUM_CASE_COMPARE 0
#endif

#include <tmx/common/platform/types/arrays.hpp>
#include <tmx/common/platform/types/bytes.hpp>
#include <tmx/common/platform/types/introspect.hpp>
#include <tmx/common/platform/types/size_of.hpp>
#include <tmx/common/platform/types/traits.hpp>

#include <tmx/common/platform/modules/tmx_module.hpp>
#include <tmx/common/platform/modules/tmx_version.hpp>

#ifndef TMX_NO_MAGIC_ENUM
#include "third_party/magic_enum.hpp"
#endif

#include <array>
#include <cstddef>
#include <optional>
#include <utility>


#ifdef MAGIC_ENUM_SUPPORTED

namespace magic_enum {
namespace detail {

/*!
 * @brief A helper structure for case-insensitve comparison
 */
template <typename _CharT = tmx::common::char_t>
struct char_equal_to_ic {
	/*!
	 * @return True if the lhs is the same character as rhs in lower case.
	 */
	static inline bool compare(_CharT lhs, _CharT rhs) {
		return ::tolower(lhs) == ::tolower(rhs);
	}

	/*!
	 * @return True if the lhs is the same character as rhs in lower case.
	 */
	bool operator()(_CharT lhs, _CharT rhs) {
		return compare(lhs, rhs);
	}
};

} /* End namespace detail */

/*!
 * @brief The module tag for the Magic Enum library
 */
struct Magic_Enum {};

} /* End namespace magic_enum */

#else
#include <algorithm>
#include <cctype>
#include <string>
#endif

namespace tmx {
namespace common {
namespace enums {

// Need to forward declare certain functions that are used later

/*!
 * @return The number of values in the enum type _E
 */
template <typename _E>
constexpr std::size_t enum_count() noexcept;

/*!
 * @return The names of all the values in the enum type _E
 */
template <typename _E>
constexpr auto enum_names() noexcept;


/*!
 * @brief Get all the values in the enum type _E
 *
 * This function helps create a compile-time array structure
 * containing all the values of the enumeration type. This can
 * be handy for use in template parameters or other constexpr
 * cases.
 *
 * This function will automatically create the array structure,
 * regardless if the Magic Enum library is supported, presuming
 * that the enum sequence function is specialized.
 *
 * @return All the values in the enum type _E
 */
template <typename _E>
constexpr std::array<_E, enum_count<_E>()> enum_values() noexcept;

/*!
 * @return The fully-qualified type identifier name for the enum type _E
 */
template <typename _E>
constexpr const_string enum_type_name() noexcept {
	static_assert(is_enum_type<_E>(), "The template parameter must be an enumeration type");

	return { type_fqname<_E>() };
}

/*!
 * @return The fully-qualified type identifier name for the enum type _E
 */
template <typename _E>
constexpr const_string enum_type_name(_E) noexcept {
	static_assert(is_enum_type<_E>(), "The argument must be an enumeration type");

	return enum_type_name<_E>();
}

/*!
 * @brief A helper function to create the enum sequence
 *
 * This converts the std::array of enum values within the
 * specified index sequence into a TMX static array of
 * type _E.
 *
 * @return The enum sequence for _E
 */
template <typename _E, typename _T, _T ... _Idx>
constexpr auto make_enum_sequence(std::integer_sequence<_T, _Idx...> const &) noexcept {
	constexpr auto values = enum_values<_E>();
	return static_array<_E, values[_Idx]...> { };
}

/*!
 * @brief Generate a static array sequence of the enumeration
 *
 * The sequence must be a TMX static array with type _E,
 * and must enumerate all possible values of the enumeration
 * from start to finish, in the order they appear in the
 * declaration.
 *
 * For compilers that support the Magic Enum library,
 * this sequence is generated automatically.  Otherwise,
 * this function must be specialized for any enum type
 * that requires compile-time dynamic use. This would
 * generally only be useful in cases such as template
 * specialization by enum,
 *
 * Any request for a enum sequence that is not pre-defined
 * and cannot be automatically generated will result in
 * a compiler error.
 *
 * @return A TMX static array of all the values of _E
 */
template <typename _E>
constexpr auto enum_sequence() noexcept {
#ifdef MAGIC_ENUM_SUPPORTED
	return make_enum_sequence<_E>(std::make_index_sequence<enum_count<_E>()> {});
#else
	return static_array<nullptr_t> { };
#endif
}

/*!
 * @return A TMX static array of all the values of _E
 */
template <typename _E>
constexpr auto enum_sequence(_E) noexcept {
	return enum_sequence<_E>();
}

/*!
 * @brief Check if the enumeration _E is valid for TMX use
 *
 * This entails ensuring that the type is truly of a C/C++
 * enum type and that there is a compile-type static
 * sequence of values of the appropriate type.
 *
 * This function is used at compile-time when trying
 * to access functions that
 *
 * @return True if the enum type is valid for TMX use
 */
template <typename _E, bool _CompileErr = false>
constexpr bool is_valid_enum() {
	constexpr bool _is_enum = is_enum_type<_E>();
	if constexpr (_CompileErr) {
			static_assert(_is_enum,
					"The template parameter must be an enumeration type");
	}

#ifdef MAGIC_ENUM_SUPPORTED
	return _is_enum;
#else
	constexpr auto _seq = enum_sequence<_E>();
	typedef typename decltype(_seq)::value_type value_type;

	constexpr bool _no_seq = std::is_same_v<nullptr_t, value_type>;
	constexpr bool _is_type = std::is_same_v<_E, value_type>;
	constexpr bool _has_vals = _seq.size > 0;

	if constexpr (_CompileErr) {
		static_assert(!_no_seq,
R"(No default values can be provided because the Magic Enum class is not support for the compiler. 
You must specialize the tmx::common::enum_sequence<>() template function to explicitly define
a compile-time sequence, or use the supplied run-time class.)");

		static_assert(_is_type,
				"The enum_sequence must be of type _E.");
		static_assert(_has_vals,
				"The enum_sequence must have at least one value.");
	}

	return _is_enum && !_no_seq && _is_type && _has_vals;
#endif
}

/*!
 * @brief A structure to wrap the constexpr functions around
 */
template <typename _E>
using is_enum_valid = std::integral_constant<bool, is_valid_enum<_E>()>;

/*!
 * @brief Check if the enumeration _E is valid for TMX use
 *
 * This entails ensuring that the type is truly of a C/C++
 * enum type and that there is a compile-type static
 * sequence of values of the appropriate type.
 *
 * This function is used at compile-time when trying
 * to access functions that
 *
 * @return True if the enum type is valid for TMX use
 */
template <typename _E, bool _CompileErr = false>
constexpr bool is_valid_enum(_E value) {
	constexpr bool _is_enum = is_enum_type<_E>();
	static_assert(_CompileErr && _is_enum,
			"The argument must be an enumeration type");

	return is_valid_enum<_E, _CompileErr>();
}

/*!
 * @brief Get the number of values in the enumeration
 *
 * For compilers that support the Magic Enum library,
 * this count is generated automically. Otherwise,
 * this function is dependent on the static array of
 * enum values returned from enum_sequence(). If no
 * sequence can be generated, if the type of the
 * array is incorrect, or if the size is 0, then a
 * compiler error will occur.
 *
 * @see #enum_sequence()
 * @return The number of values in _E
 */
template <typename _E>
constexpr std::size_t enum_count() noexcept {
	is_valid_enum<_E, true>();

#ifdef MAGIC_ENUM_SUPPORTED
	return magic_enum::enum_count<_E>();
#else
	constexpr auto _seq = enum_sequence<_E>();
	return _seq.size;
#endif
}

/*!
 * @return The number of values in _E
 */
template <typename _E>
constexpr std::size_t enum_count(_E value) noexcept {
	is_valid_enum<_E, true>(value);
	return enum_count<_E>();
}

/*!
 * @brief Get all the values in the enumeration
 *
 * This should be a std::array with enum_count<_E>()
 * values of type _E.
 *
 * For compilers that support the Magic Enum library,
 * this array is generated automically. Otherwise,
 * this function is dependent on the static array of
 * enum values returned from enum_sequence(). If no
 * sequence can be generated, if the type of the
 * array is incorrect, or if the size is 0, then a
 * compiler error will occur.
 *
 * @see #enum_count<_E>()
 * @return An array of all the values of _E
 */
template <typename _E>
constexpr std::array<_E, enum_count<_E>()> enum_values() noexcept {
	is_valid_enum<_E, true>();

#ifdef MAGIC_ENUM_SUPPORTED
	return magic_enum::enum_values<_E>();
#else
	constexpr auto _seq = enum_sequence<_E>();
	return _seq.array;
#endif
}

/*!
 * @return An array of all the values of _E
 */
template <typename _E>
constexpr auto enum_values(_E value) noexcept {
	is_valid_enum<_E, true>(value);
	return enum_values<_E>();
}

/*!
 * @return Return the array of default names
 */
template <class _C, std::size_t ... _I>
constexpr auto _make_string_array(_C const &, std::index_sequence<_I...> const &) {
	constexpr _C _proto {};
	return std::array<const_string, sizeof...(_I)> {
		(make_string<static_cast<std::size_t>(_proto.at(_I))>().c_str())...,
	};
}
/*!
 * @brief Get all the names of the values in the enumeration
 *
 * This should be a std::array with enum_count<_E>()
 * values of constant strings.
 *
 * There are a number of use cases for reflective name conversion
 * of enums at compile-time, however C++ does not by default support
 * this capability. While TMX does not specifically require this
 * feature, it may be handy in certain cases.
 *
 * The Magic Enum library use pretty printed function signatures
 * that spit out the enum names, but this feature is limited
 * to a few compilers, namely clang, gcc, and msvc, and further
 * limited to newer releases of those compilers. If this
 * capability is available, then TMX utilizes it and this
 * function will automatically generate the array of names.
 *
 * For compilers that support the Magic Enum library, this array
 * is generated automically. Otherwise, this function is dependent
 * on the static array of enum values returned from enum_sequence().
 * If no sequence can be generated, if the type of the array is
 * incorrect, or if the size is 0, then a compiler error will occur.
 *
 * In addition, this function may be specialized to include
 * customized enum names, even if they cannot be generated
 * automatically. But, if no specialization exists, then the
 * default implementation will automatically construct a stringified
 * version of each numeric value in _E, which at least permits a
 * conversion to a textual string for output purposes.
 *
 * @see #enum_sequence<_E>()
 * @return An array of all the names of the values of _E
 */
template <typename _E>
constexpr auto enum_names() noexcept {
	is_valid_enum<_E, true>();

#ifdef MAGIC_ENUM_SUPPORTED
	return magic_enum::enum_names<_E>();
#else
	return _make_string_array(enum_sequence<_E>(), std::make_index_sequence<enum_count<_E>()> {});
#endif
}

/*!
 * @return An array of all the names of the values of _E
 */
template <typename _E>
constexpr auto enum_names(_E value) noexcept {
	is_valid_enum<_E, true>(value);
	return enum_names<_E>();
}

/*!
 * @return The static array of entries
 */
template <typename _E, std::size_t ... _I>
constexpr auto _make_enum_entries(std::index_sequence<_I...> const &) {
	is_valid_enum<_E, true>();

	typedef std::pair<_E, const_string> type;

	constexpr auto _seq = enum_sequence<_E>();
	constexpr auto _names = enum_names<_E>();
	return std::array<type, sizeof...(_I)> {
		std::make_pair(_seq.array[_I], _names[_I])...
	};
}

/*!
 * @brief Get the value-name pairs of all the enumeration entries
 *
 * The first entry in the pair is the enumeration value, while
 * the second is the name as a compile-time string view.
 *
 * For compilers that support the Magic Enum library, the array
 * is generated automatically. Otherwise, this function is dependent
 * on the static array of enum values returned from enum_sequence().
 * If no sequence can be generated, if the type of the array is
 * incorrect, or if the size is 0, then a compiler error will occur.
 *
 * @see #std::pair
 * @return The value-name pairs for the enumeration _E
 */
template <typename _E>
constexpr auto enum_entries() noexcept {
	is_valid_enum<_E, true>();

#ifdef MAGIC_ENUM_SUPPORTED
	return magic_enum::enum_entries<_E>();
#else
	return _make_enum_entries<_E>(std::make_index_sequence<enum_count<_E>()> {});
#endif
}

/*!
 * @return The value-name pairs for the enumeration _E
 */
template <typename _E>
constexpr auto enum_entries(_E value) noexcept {
	is_valid_enum<_E, true>(value);
	return enum_entries<_E>();
}

/*!
 * @brief Get the array index for the enumeration value _V
 *
 * Given the static array sequence for the enumeration,
 * this returns the index at which _V occurs. This
 * function should always be evaluated at compile-time.
 * Therefore, if the value does not really exist in the
 * enumeration, then a compiler error will occur.
 *
 * For compilers that support the Magic Enum library, the array
 * is generated automatically. Otherwise, this function is dependent
 * on the static array of enum values returned from enum_sequence().
 * If no sequence can be generated, if the type of the array is
 * incorrect, or if the size is 0, then a compiler error will occur.
 *
 * @return The order at which _V occurs in the enumeration
 */
template <auto _V, std::size_t _I = 0>
constexpr std::size_t enum_index() noexcept {
	is_valid_enum<decltype(_V), true>();

#ifdef MAGIC_ENUM_SUPPORTED
	return magic_enum::enum_index<_V>();
#else
	constexpr auto _seq = enum_sequence<decltype(_V)>();
	static_assert(_I < _seq.size, "There is no index for the specified value _V");

	if constexpr (_V == _seq.array[_I])
		return _I;
	else
		return enum_index<_V, _I+1>();
#endif
}

/*!
 * @brief Get the array index for the enumeration value _V
 *
 * Given the static array sequence for the enumeration,
 * this returns the index at which value occurs. This
 * function would typically be evaluated at run-time
 * Therefore, if the value does not really exist in the
 * enumeration, it will return a const_string::npos index,
 * indicating that the value does not exist.
 *
 * For compilers that support the Magic Enum library, the array
 * is generated automatically. Otherwise, this function is dependent
 * on the static array of enum values returned from enum_sequence().
 * If no sequence can be generated, if the type of the array is
 * incorrect, or if the size is 0, then a compiler error will occur.
 *
 * @param The value to check for
 * @return The order at which the value occurs in the enumeration
 */
template <typename _E>
constexpr std::size_t enum_index(_E value) noexcept {
	is_valid_enum<_E, true>(value);

#ifdef MAGIC_ENUM_SUPPORTED
	return magic_enum::enum_index<_E>(value).value_or(const_string::npos);
#else
	std::size_t index = 0;
	for (auto val: enum_values<_E>()) {
		if (value == val) return index;
		index++;
	}

	return const_string::npos;
#endif
}

/*!
 * @return The integer equivalent for the enumeration value
 */
template <typename _E>
constexpr std::underlying_type_t<_E> enum_integer(_E value) noexcept {
	is_valid_enum<_E, true>(value);

#ifdef MAGIC_ENUM_SUPPORTED
	return magic_enum::enum_integer<_E>(value);
#else
	return static_cast<std::underlying_type_t<_E> >(value);
#endif
}

/*!
 * @brief Get the name for the given enumeration value
 *
 * For compilers that support the Magic Enum library, the evaluation
 * can be done at compile-time. Otherwise, this function is dependent
 * on the static array of enum values returned from enum_sequence().
 * If no sequence can be generated, if the type of the array is
 * incorrect, or if the size is 0, then a compiler error will occur.
 * However, regardless, the evaluation will be done at run-time.
 *
 * @param The value to check
 * @return The enumeration name of the value
 */
template <typename _E>
constexpr std::optional<_E> enum_cast(std::underlying_type_t<_E> value) noexcept {
	is_valid_enum<_E, true>();

#ifdef MAGIC_ENUM_SUPPORTED
	return magic_enum::enum_cast<_E>(value);
#else
	constexpr auto values = enum_values<_E>();
	for (std::size_t index = 0; index < values.size(); index++) {
		if (value == enum_integer<_E>(values[index]))
			return { values[index] };
	}

	return { };
#endif
}

/*!
 * @brief Get the name for the given enumeration name
 *
 * For compilers that support the Magic Enum library, the evaluation
 * can be done at compile-time. Otherwise, this function is dependent
 * on the static array of enum values returned from enum_sequence().
 * If no sequence can be generated, if the type of the array is
 * incorrect, or if the size is 0, then a compiler error will occur.
 * However, regardless, the evaluation will be done at run-time.
 *
 * @param The value to check
 * @return The enumeration name of the value
 */
template <typename _E>
constexpr std::optional<_E> enum_cast(const_string name, bool caseSensitive = TMX_ENUM_CASE_COMPARE) noexcept {
	is_valid_enum<_E, true>();

#ifdef MAGIC_ENUM_SUPPORTED
	if (caseSensitive)
		return magic_enum::enum_cast<_E>(name);
	else
		return magic_enum::enum_cast<_E>(name, magic_enum::detail::char_equal_to_ic<> {});
#else
	constexpr auto names = enum_names<_E>();
	constexpr auto values = enum_values<_E>();

	std::string _name { name };
	if (!caseSensitive)
		std::transform(_name.begin(), _name.end(), _name.begin(), ::tolower);

	for (std::size_t i = 0; i < names.size(); i++) {
		std::string _other { names[i] };
		if (!caseSensitive)
			std::transform(_other.begin(), _other.end(), _other.begin(), ::tolower);

		if (_name == _other) return { values[i] };
	}

	return {};
#endif
}


/*!
 * @brief Checks whether the enumeration type contains the given value
 *
 * For compilers that support the Magic Enum library, the evaluation
 * can be done during compile-time. Otherwise, this function is dependent
 * on the static array of enum values returned from enum_sequence().
 * If no sequence can be generated, if the type of the array is
 * incorrect, or if the size is 0, then a compiler error will occur.
 * However, regardless, the evaluation will be done at run-time.
 *
 * @param The value to check
 * @return True if the enumeration type contains the value
 */
template <typename _E>
constexpr bool enum_contains(_E value) noexcept {
	is_valid_enum<_E, true>(value);

#ifdef MAGIC_ENUM_SUPPORTED
	return magic_enum::enum_contains<_E>(value);
#else
	auto _index = enum_index(value);
	return _index < enum_count<_E>();
#endif
}


/*!
 * @brief Checks whether the enumeration type contains the given integer value
 *
 * For compilers that support the Magic Enum library, the evaluation
 * can be done during compile-time. Otherwise, this function is dependent
 * on the static array of enum values returned from enum_sequence().
 * If no sequence can be generated, if the type of the array is
 * incorrect, or if the size is 0, then a compiler error will occur.
 * However, regardless, the evaluation will be done at run-time.
 *
 * @param The value to check
 * @return True if the enumeration type contains the integer value
 */
template <typename _E>
constexpr bool enum_contains(std::underlying_type_t<_E> value) noexcept {
	is_valid_enum<_E, true>();

#ifdef MAGIC_ENUM_SUPPORTED
	return magic_enum::enum_contains<_E>(value);
#else
	return enum_cast<_E>(value).has_value();
#endif
}

/*!
 * @brief Checks whether the enumeration type contains the given name
 *
 * For compilers that support the Magic Enum library, the evaluation
 * can be done during compile-time. Otherwise, this function is dependent
 * on the static array of enum values returned from enum_sequence().
 * If no sequence can be generated, if the type of the array is
 * incorrect, or if the size is 0, then a compiler error will occur.
 * However, regardless, the evaluation will be done at run-time.
 *
 * @param The value to check
 * @return True if the enumeration type contains the name
 */
template <typename _E>
constexpr bool enum_contains(const_string name, bool caseSensitive = TMX_ENUM_CASE_COMPARE) noexcept {
	is_valid_enum<_E, true>();

#ifdef MAGIC_ENUM_SUPPORTED
	if (caseSensitive)
		return magic_enum::enum_contains<_E>(name);
	else
		return magic_enum::enum_contains<_E>(name, magic_enum::detail::char_equal_to_ic<> {});
#else
	return enum_cast<_E>(name, caseSensitive).has_value();
#endif
}

/*!
 * @brief Get the name for the given enumeration value
 *
 * For compilers that support the Magic Enum library, the evaluation
 * can be done automatically. Otherwise, this function is dependent
 * on the static array of enum values returned from enum_sequence().
 * If no sequence can be generated, if the type of the array is
 * incorrect, or if the size is 0, then a compiler error will occur.
 * However, regardless, the evaluation will be done at compile-time.
 *
 * @return The enumeration name of _V
 */
template <auto _V>
constexpr const_string enum_name() noexcept {
	is_valid_enum<decltype(_V), true>();

#ifdef MAGIC_ENUM_SUPPORTED
	return { magic_enum::enum_name<_V>() };
#else
	constexpr auto names = enum_names<decltype(_V)>();
	return names[enum_index<_V>()];
#endif
}

/*!
 * @brief Get the name for the given enumeration value
 *
 * For compilers that support the Magic Enum library, the evaluation
 * can be done at compile-time. Otherwise, this function is dependent
 * on the static array of enum values returned from enum_sequence().
 * If no sequence can be generated, if the type of the array is
 * incorrect, or if the size is 0, then a compiler error will occur.
 * However, regardless, the evaluation will be done at run-time.
 *
 * @param The value to check
 * @return The enumeration name of the value
 */
template <typename _E>
constexpr const_string enum_name(_E value) noexcept {
	is_valid_enum<_E, true>(value);

#ifdef MAGIC_ENUM_SUPPORTED
	return { magic_enum::enum_name<_E>(value) };
#else
	constexpr auto names = enum_names<_E>();
	auto index = enum_index(value);
	if (index == const_string::npos) return empty_string();
	return names[index];
#endif
}

/*!
 * @brief Get the enumeration value for the given index
 *
 * For compilers that support the Magic Enum library, the evaluation
 * can be done automatically. Otherwise, this function is dependent
 * on the static array of enum values returned from enum_sequence().
 * If no sequence can be generated, if the type of the array is
 * incorrect, or if the size is 0, then a compiler error will occur.
 * However, regardless, the evaluation will be done at compile-time.
 *
 * @return The enumeration value at _I
 */
template <typename _E, std::size_t _I>
constexpr _E enum_value() noexcept {
	is_valid_enum<_E, true>();

#ifdef MAGIC_ENUM_SUPPORTED
	return magic_enum::enum_value<_E, _I>();
#else
	constexpr auto _seq = enum_sequence<_E>();
	static_assert(_I > 0 && _I < _seq.size, "The index is outside the range of the enum values for _E");
	return _seq.at(_I);
#endif
}

/*!
 * @brief Get the enumeration value for the given index
 *
 * For compilers that support the Magic Enum library, the evaluation
 * can be done at compile-time. Otherwise, this function is dependent
 * on the static array of enum values returned from enum_sequence().
 * If no sequence can be generated, if the type of the array is
 * incorrect, or if the size is 0, then a compiler error will occur.
 * However, regardless, the evaluation will be done at run-time.
 *
 * @param The index to check
 * @return The enumeration value at the index
 */
template <typename _E>
constexpr _E enum_value(std::size_t index) noexcept {
	is_valid_enum<_E, true>();

#ifdef MAGIC_ENUM_SUPPORTED
	return magic_enum::enum_value<_E>(index);
#else
	constexpr auto values = enum_values<_E>();
	if (index < values.size())
		return values[index];

	return static_cast<_E>(index);
#endif
}

} /* End namespace enums */

#ifdef MAGIC_ENUM_SUPPORTED

template <>
struct tmx_module_version<magic_enum::Magic_Enum> {
    using version = tmx_version_t<MAGIC_ENUM_VERSION_MAJOR, MAGIC_ENUM_VERSION_MINOR, MAGIC_ENUM_VERSION_PATCH>;
};

#endif

} /* End namespace common */
} /* End namespace tmx */

#include <boost/preprocessor.hpp>

#define TMX_ENUM_GENERATE_ID(r, data, i, elem) BOOST_PP_COMMA_IF(i) data::elem
#define TMX_ENUM_GENERATE_NAME(r, data, i, elem) BOOST_PP_COMMA_IF(i) TMX_CPP_QUOTE(elem)
#define TMX_ENUM_GENERATE_ALL_IDS(name, ...) BOOST_PP_SEQ_FOR_EACH_I(TMX_ENUM_GENERATE_ID, name, BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__))
#define TMX_ENUM_GENERATE_ALL_NAMES(name, ...) BOOST_PP_SEQ_FOR_EACH_I(TMX_ENUM_GENERATE_NAME, name, BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__))

/*!
 * @brief A macro to help generate the enum_sequence and enum_names specializations
 *
 * The assumption here is that the enumeration class was already created, since doing that
 * requires namespaces. Therefore, this macrp will only build the necessary enum_sequence
 * function specialization that will enumerate all the values in the variadic list.
 * Additionally, those names are quoted to automatically generate the list of names
 * for the values.
 *
 * Usage: TMX_ENUM_SEQUENCE(com::example::EnumTypeName, FirstItem, SecondItem, ...)
 *
 * The downside with this is that at least in some compilers, the __VA_ARGS__ macro
 * only expands up to 64 elements. For enums that have more than 64 elements, it
 * is recommended to build the specializations using the TMX_ENUM_GENERATE_ALL_*
 * macros.
 */
#ifdef MAGIC_ENUM_SUPPORTED
#define TMX_ENUM_SEQUENCE(name, ...)
#else
#define TMX_ENUM_SEQUENCE(name, ...) \
namespace tmx { namespace common { \
template <> constexpr auto enum_sequence<name>() noexcept { \
	return static_array<name, TMX_ENUM_GENERATE_ALL_IDS(name, __VA_ARGS__)> {}; \
} \
template <> constexpr auto enum_names<name>() noexcept { \
	return std::array<const_string, enum_count<name>()> { TMX_ENUM_GENERATE_ALL_NAMES(name, __VA_ARGS__) }; \
} \
}}

#endif

#endif /* PLATFORM_INCLUDE_TMX_COMMON_PLATFORM_ENUMERATION_ENUM_HPP_ */
