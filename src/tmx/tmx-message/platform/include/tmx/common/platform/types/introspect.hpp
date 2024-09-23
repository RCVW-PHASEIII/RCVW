/*!
 * Copyright (c) 2021 Battelle Memorial Institute
 *
 * All Rights Reserved.
 *
 * @file introspect.hpp
 *
 *  Created on: Jun 14, 2021
 *      @author: Gregory M. Baumgardner
 */

#ifndef PLATFORM_INCLUDE_TMX_COMMON_PLATFORM_TYPES_INTROSPECT_HPP_
#define PLATFORM_INCLUDE_TMX_COMMON_PLATFORM_TYPES_INTROSPECT_HPP_

#include <tmx/common/platform/compiler/cpp.hpp>

#include <tmx/common/platform/iteration/sequence.hpp>
#include <tmx/common/platform/types/arrays.hpp>
#include <tmx/common/platform/types/bytes.hpp>

#include <utility>

namespace tmx {
namespace common {

/*!
 * @brief A structure that holds the compile-time string equivalent for the specified type
 *
 * This operation used to use a type demangling operation, but the trick discovered in the magic_enum
 * code using the compiler pretty function enabled this to become a fully compile-time operation.
 *
 * However, there is no guarantee that such a trick works for every compiler or in all situations,
 * therefore the introspection class is has a version argument that allows multiple implementations and
 * can then be selected at compile time for the proper situation. This extra argument also helps the
 * default (version 1) implementation in that parsing the pretty function becomes easier with multiple
 * template parameters.
 *
 * Additionally, this structure uses the TMX platform types to create a static array of character
 * values that can be appended upon like a StringBuilder type in order to make compile-time
 * constant strings made up of type name details. This is very handy because it allows for type names
 * to be built at compile time using constant expressions, such as a size values. In other words, without
 * this class, one could not easily distinguish between the int<15> and int<16> or the array<int> and array<uint>
 * structure types solely by the name.
 *
 * @param _T The type to introspect
 * @param _V The introspection implementation version number which defaults to 1
 */
template <typename _T, std::uint16_t _V = 1>
class introspection_t {
public:
	/*!
	 * @brief The underlying type
	 */
	typedef _T type;

	/*!
	 * @brief A type used to check if the structure is an introspection
	 */
	typedef std::true_type introspect;

private:
	/*!
	 * @return The precise function signature of this method with template type substitution
	 */
	static constexpr const char * pretty_signature() {
		return
#ifdef TMX_PRETTY_FUNCTION
	    ( TMX_PRETTY_FUNCTION );
#else
	    ( "" );
#endif
	}

public:
	/*!
	 * @brief A variable to hold the full pretty signature
	 *
	 * Note that a constant string view has more useful functions for
	 * extracting the lengths, plus searching and parsing the
	 * sequence.
	 *
	 * An example signature from GCC 9 looks like:
	 *
	 * static constexpr const char* tmx::common::introspection_t<_T, _V>::pretty_signature()
	 * [with _T = com::example::data; short unsigned int _V = 1]
	 *
	 * The template type parameters are enclosed in square brackets []
	 * and are semi-colon separated following an equal sign. Therefore,
	 * it becomes trivial to parse the compile-time literal to determine
	 * the first template parameter type, which is _T.
	 */
	static constexpr auto signature = const_string { pretty_signature() };

	static_assert(signature.size() > 0,
			"This compiler does not seem to support printing function signature. Find a compile time type introspection for your specific compiler.");

protected:
	/*!
	 * @brief A string literal for where to find the start of the template type information
	 *
	 * This should be set by the TMX_PRETTY_FUNCTION_PARSE_START macro.
	 *
	 * For the GCC example, this should be "[with _T = "
	 *
	 * @see #signature
	 */
	static constexpr const char* _start =
#ifdef TMX_PRETTY_FUNCTION_PARSE_START
			TMX_PRETTY_FUNCTION_PARSE_START;
#else
			nullptr;
#endif

	/*!
	 * @brief A string literal for where to find the end of the template type information
	 *
	 * This should be set by the TMX_PRETTY_FUNCTION_PARSE_END macro.
	 *
	 * For the GCC example, this should be the semicolon separator ";"
	 *
	 * @see #signature
	 */
	static constexpr const char* _end =
#ifdef TMX_PRETTY_FUNCTION_PARSE_END
			TMX_PRETTY_FUNCTION_PARSE_END;
#else
			nullptr;
#endif

	static_assert(_start != nullptr,
			"Something went wrong with the compiler setup. Please set TMX_PRETTY_FUNCTION_PARSE_START.");
	static_assert(_end != nullptr,
			"Something went wrong with the compiler setup. Please set TMX_PRETTY_FUNCTION_PARSE_END.");

	static constexpr auto sFind = signature.find(_start);
	static constexpr auto sIdx = sFind + const_string(_start).size();
	static constexpr auto eIdx = signature.find(_end, sIdx) - 1;
	static_assert(eIdx > 0 && eIdx != const_string(_start).length(),
			"Compiler error in printing function signature: Cannot find the template end");

	static constexpr auto trim = (signature.substr(sIdx, 6) == "class " ? 6 :
		(signature.substr(sIdx, 7) == "struct " ? 7 : 0));

public:
	static constexpr const char * name_s = signature.data() + sIdx + trim;

	/*!
	 * @brief The fully-qualified name of the _T type
	 *
	 * This is built as a constant string so it may also
	 * be searched and parsed. The constant character
	 * sequence used in this string view is the entire
	 * function signature. Since this itself is a constant
	 * compile-time structure unique to the specific type,
	 * this attribute may be used in some container applications,
	 * although that is discouraged.
	 *
	 * @see #signature
	 */
	static constexpr const_string name { name_s, eIdx - sIdx + 1 };
};

// Forward declare the one function that must be implemented
template <typename _T, std::uint16_t _V>
constexpr auto _fqname(introspection_t<_T, _V> const &) noexcept;

} /* End namespace common */
} /* End namespace tmx */

/*!
 * @brief Include the specific introspection implementation
 *
 * While all of the implementation utilize a pretty-printed signature
 * to obtain the type name, building the static character type
 * array structure is done differently.
 *
 * Once the fully-qualified name is obtained, the rest of the
 * functions below can dynamically pull out the namespace and
 * short name portion.
 *
 * This include must also set the default TMX_INTRSOPECTON_VERSION
 * macro.
 */
#ifdef TMX_PRETTY_FUNCTION_NOT_CONSTEXPR
#include "impl/chararray_signature_introspect.hpp"
#else
#include "impl/constexpr_signature_introspect.hpp"
#endif

namespace tmx {
namespace common {

/*!
 * @brief An alias to the introspection of _T using the correct implementation
 */
template <typename _T>
using introspection = introspection_t<std::decay_t<_T>, TMX_INTROSPECTION_VERSION>;

/*!
 * @brief Use a C++ style of introspection on the given type
 *
 * This function uses the introspection class to handle the compile-time conversion,
 * which is only needed in special cases. In order to append strings to the
 * type name, for example, use the value field in this introspection class directly.
 *
 * @see introspection
 * @param Some type
 * @returns The fully-qualified name of T as a C-style character string
 */
template <typename _T>
constexpr auto introspect() noexcept {
	return introspection<_T> {};
}

/*!
 * @brief Use a C++ style of introspection on the type of the given argument
 *
 * This function uses the introspection class to handle the compile-time conversion,
 * which is only needed in special cases. In order to append strings to the
 * type name, for example, use the value field in this introspection class directly.
 *
 * @see introspection
 * @param Some type
 * @returns The fully-qualified name of _T as a C-style character string
  */
template <typename _T>
constexpr auto introspect(_T const &) noexcept {
	return introspect<_T>();
}

/*!
 * @return The static character type array equivalent of the fully-qualified name of _T
 */
template <typename _T>
constexpr auto introspect_fqname() noexcept {
	return _fqname(introspect<_T>());
}

/*!
 * @brief Determine the fully-qualified identifier name of the given type
 *
 * This functions uses the introspection class to handle the compile-time conversion,
 * but returns the fully-qualified identifier name as a constant string view type.
 *
 * @param Some type
 * @returns The fully-qualified name of _T
 */
template <typename _T>
constexpr const_string type_fqname() noexcept {
	constexpr auto _name = introspect_fqname<_T>();
	return { _name.c_str(), _name.size };
}

/*!
 * @brief Determine the fully-qualified identifier name of the type of the given argument
 *
 * This functions uses the introspection class to handle the compile-time conversion,
 * but returns the fully-qualified identifier name as a constant string view type.
 *
 * @param Some type
 * @returns The fully-qualified name of _T
 */
template <typename _T>
constexpr const_string type_fqname(_T const &) noexcept {
	return type_fqname<_T>();
}

/*!
 * @return The static character type array equivalent of the fully-qualified namespace of _T
 */
template <typename _T>
constexpr auto introspect_namespace() noexcept {
	typedef introspection<_T> type;
	constexpr valid_id_characters _idc;
	constexpr std::size_t _lIdx = type::name.find_first_not_of(_idc.c_str());
	constexpr std::size_t _tIdx = type::name.substr(0, _lIdx).find_last_of(":");
	if constexpr (_tIdx > 0 && _tIdx < const_string::npos) {
		return make_array(introspect_fqname<_T>(),
				make_index_sequence<_tIdx - 1>());
	} else {
		return TMX_STATIC_STRING("");
	}
}

/*!
 * @brief Determine the fully-qualified namespace of the given type
 *
 * This functions uses the introspection class to handle the compile-time conversion,
 * but returns the fully-qualified namespace as a constant string view type.
 *
 * @param Some type
 * @returns The fully-qualified namespace of _T
 */
template <typename _T>
constexpr const_string type_namespace() noexcept {
	constexpr auto _name = introspect_namespace<_T>();
	return { _name.c_str(), _name.size };
}

/*!
 * @brief Determine the fully-qualified namespace of the type of the given argument
 *
 * This functions uses the introspection class to handle the compile-time conversion,
 * but returns the fully-qualified namespace as a constant string view type.
 *
 * @param Some type
 * @returns The fully-qualified namespace of _T
 */
template <typename _T>
constexpr const_string type_namespace(_T const &) noexcept {
	return type_namespace<_T>();
}


/*!
 * @return The static character type array equivalent of the non-qualified name of _T
 */
template <typename _T>
constexpr auto introspect_short_name() noexcept {
	typedef introspection<_T> type;
	constexpr valid_id_characters _idc;
	constexpr std::size_t _lIdx = type::name.find_first_not_of(_idc.c_str());
	constexpr std::size_t _tIdx = type::name.substr(0, _lIdx).find_last_of(":");
	if constexpr (_tIdx > 0 && _tIdx < const_string::npos) {
		return make_array(introspect_fqname<_T>(),
				make_index_sequence<type::name.length() - _tIdx - 1, _tIdx + 1>());
	} else {
		return introspect_fqname<_T>();;
	}
}


/*!
 * @brief Determine the identifier name of the given type within its namespace
 *
 * This functions uses the introspection class to handle the compile-time conversion,
 * but returns the non-qualified name as a constant string view type.
 *
 * @param Some type
 * @returns The non-qualified name of _T
 */
template <typename _T>
constexpr const_string type_short_name() noexcept {
	constexpr auto _name = introspect_short_name<_T>();
	return { _name.c_str(), _name.size };
}

/*!
 * @brief Determine the identifier name of the type of the given argument within its namespace
 *
 * This functions uses the introspection class to handle the compile-time conversion,
 * but returns the non-qualified name as a constant string view type.
 *
 * @param Some type
 * @returns The non-qualified name of _T
 */
template <typename _T>
constexpr const_string type_short_name(_T const &) noexcept {
	return type_short_name<_T>();
}

/*!
 * @return A compile-time string to the default types namespace in case one is not known
 */
constexpr auto default_namespace_v() noexcept {
	typedef introspection<int> _test_t;
	constexpr auto _types = TMX_STATIC_STRING("::types");
	return concat(introspect_namespace<_test_t>(), _types);
}

/*!
 * @return A constant character string for the default types namespace in case one is not known
 */
constexpr const_string default_namespace() noexcept {
	return { default_namespace_v().c_str() };
}

/*!
 * @brief Determine if the specified type matches the type id
 *
 * Use a C++ style of introspection in order to check if the type name matches the supplied argument.
 *
 * @param fqname The fully-qualified identifier name for the type
 * @returns True if the name matches, False otherwise
 */
template <typename _T>
constexpr bool is_type_fqname(const char *fqname) noexcept {
	return (type_fqname<_T>() == fqname);
}

/*!
 * @brief Determine if the specified type matches the type id
 *
 * Use a C++ style of introspection in order to check if the type name matches the supplied argument.
 *
 * @param fqname The fully-qualified identifier name for the type
 * @param type A reference to some data of type T
 * @returns True if the name matches, False otherwise
 */
template <typename _T>
constexpr bool is_type_fqname(const char *fqname, const _T &type) noexcept {
	return is_type_fqname<_T>(fqname);
}

/*!
 * @brief Determine if the specified type matches the namespace
 *
 * Use a C++ style introspection in order to check if the fully-qualified namespace of the type
 * matches the supplied argument.
 *
 * @param nmspace The namespace for the type
 * @returns True if the namespace matches, False otherwise
 */
template <typename _T>
constexpr bool is_type_namespace(const char *nmspace) noexcept {
	return type_namespace<_T>() == nmspace;
}

/*!
 * @brief Determine if the specified type matches the namespace
 *
 * Use a C++ style introspection in order to check if the fully-qualified namespace of the type
 * matches the supplied argument.
 *
 * @param nmspace The namespace for the type
 * @param type A reference to some data of type T
 * @returns True if the namespace matches, False otherwise
 */
template <typename _T>
constexpr bool is_type_namespace(const char *nmspace, const _T &type) noexcept {
	return is_type_namespace<_T>(nmspace);
}

/*!
 * @brief Determine if the specified type matches the name
 *
 * Use a C++ style introspection in order to determine the non-qualified (or short) name of the type
 * matches supplied argument.
 *
 * @param name A name for the type
 * @returns True if the name matches, False otherwise
 */
template <typename _T>
constexpr bool is_type_short_name(const char *name) noexcept {
	return type_short_name<_T>() == name;
}

/*!
 * @brief Determine if the specified type matches the name
 *
 * Use a C++ style introspection in order to determine the non-qualified (or short) name of the type
 * matches supplied argument.
 *
 * @param name A name for the type
 * @param type A reference to some data of type T
 * @returns True if the name matches, False otherwise
 */
template <typename _T>
constexpr bool is_type_short_name(const char *name, const _T &type) noexcept {
	return is_type_short_name<_T>(name);
}

/*!
 * @brief Determine if the specified type matches the namespace and name
 *
 * Use a C++ style of introspection in order to check if both the namespace and non-qualified (short) name matches the
 * respective supplied arguments.
 *
 * @param nmspace The namespace for the type
 * @param name A name for the type
 * @returns True if the name matches, False otherwise
 */
template <typename _T>
constexpr bool is_type_name(const char *nmspace, const char *name) noexcept {
	return is_type_namespace<_T>(nmspace) && is_type_short_name<_T>(name);
}

/*!
 * @brief Determine if the specified type matches the namespace and name
 *
 * Use a C++ style of introspection in order to check if both the namespace and non-qualified (short) name matches the
 * respective supplied arguments.ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789
 *
 * @param nmspace The namespace for the type
 * @param name A name for the type
 * @param type A reference to some data of type T
 * @returns True if both the namespace and name match, False otherwise
 */
template <typename _T>
constexpr bool is_type_name(const char *nmspace, const char *name, const _T &type) noexcept {
	return is_type_name<_T>(nmspace, name);
}

} /* End namespace common */
} /* End namespace tmx */


#endif /* PLATFORM_INCLUDE_TMX_COMMON_PLATFORM_TYPES_INTROSPECT_HPP_ */
