/*
 * Copyright (c) 2022 Battelle Memorial Institute
 *
 * All Rights Reserved.
 *
 * @file default.hpp
 *
 * The TMX platform header provides a common include
 * file that specifically sets up any TMX related
 * structures, macros or other compiler-specific
 * implementations.
 *
 * It is expected that this include is sufficient
 * to be used by any other TMX header to ensure
 * that platform or compiler-specific implementations
 * can be accessed.
 *
 * Supported features, however, are typically included
 * under headers from other sub-directories based on
 * the feature.
 *
 *  Created on: Oct 5, 2022
 *      @author: Gregory M. Baumgardner
 */

#ifndef TMX_PLATFORM_INCLUDE
#define TMX_PLATFORM_INCLUDE

/*!
 * @brief Check compiler support
 *
 * The minimum C++ standard level is version 17.
 *
 * @see https://en.cppreference.com/w/cpp/17
 */
static_assert(
#ifdef _MSVC_LANG
	_MSVC_LANG
#else
	__cplusplus
#endif
	>= 201703L, "Compiling TMX minimally requires C++ 17 support");

#define TMX_CONSTEXPR_FN constexpr

#if __has_include(<unistd.h>)
#include <unistd.h>
#define TMX_UX_POSIX
#endif

// Include the compiler support headers
#include <tmx/common/platform/compiler/cpp.hpp>

// Include byte ordering help
#include <tmx/common/platform/endianness/endian.hpp>
#include <tmx/common/platform/endianness/byte_order.hpp>

// Include some basic type information constructs
#include <tmx/common/platform/types/arrays.hpp>
#include <tmx/common/platform/types/bytes.hpp>
#include <tmx/common/platform/types/traits.hpp>
#include <tmx/common/platform/types/size_of.hpp>
#include <tmx/common/platform/types/introspect.hpp>
#include <tmx/common/platform/types/tuple.hpp>

// Function support
#include <tmx/common/platform/functional/functional.hpp>

// Enum feature support
#include <tmx/common/platform/enumeration/enum.hpp>

// Cursor iteration
#include <tmx/common/platform/iteration/cursor.hpp>

// Byte strings
#include <tmx/common/platform/types/byte_string.hpp>

// Basic C++ 17 type support
#include <any>
#include <filesystem>
#include <optional>
#include <variant>

namespace tmx {
namespace common {

/*!
 * @brief This the default platform tag for TMX
 *
 * The TMX platform is comprised of operations that may
 * be specific to a standard feature set for C++
 * compilers. For example, this default platform
 * expects standard C++ 17 compiler support. This
 * is primarily to support features such as:
 *
 * - constexpr constructors
 * - constexpr functions
 * - constexpr conditional expressions
 * - literal-backed string views
 * - informative function signatures
 *
 * Some great new tools are now available with the
 * addition of these features. For example, the
 * magic_enum library is available which uses the
 * function signatures in certain compilers and the
 * string views to automatically create a static
 * array of the enum names for any type. This is
 * super useful for TMX in order to encode the
 * enumeration values as names.
 *
 * That said, it is conceivable that TMX may be
 * operational within other feature sets, such as
 * standard C++ 14 with GNU extensions. However,
 * this setup header will not work and a new one must
 * therefore be created.
 *
 * Likewise, if only a sub-set of TMX is required, say
 * for an embedded system, then custom platform header
 * should be created to ensure only the necessary
 * features are included, and such features like the
 * magic_enum can be excluded.
 *
 * There is an attempt to not rely on any specific
 * C++ features outside of the platform, i.e. any
 * file that starts with Tmx. This is in order to
 * prevent having to port the main TMX code when
 * moving to a new compiler. Of course, this is
 * not guaranteed in any way, so care must be taken
 * to verify TMX with a non-standard platform.
 *
 * The tmx_platform_module tag must ultimately point
 * to the platform structure being used
 */
struct tmx_platform_cpp17 {};

/*!
 * @brief A type alias for the TMX platform tag
 */
using tmx_platform_module = tmx_platform_cpp17;

/*!
 * @brief Using the std::any type
 */
using any = std::any;

template <class _T>
_T any_cast(const any &operand) {
    return std::any_cast<_T>(operand);
}

template <class _T>
_T any_cast(any &operand) {
    return std::any_cast<_T>(operand);
}

template <class _T>
const _T* any_cast(const any *operand) noexcept {
	return std::any_cast<_T>(operand);
}

template <class _T>
_T* any_cast(any *operand) noexcept {
	return std::any_cast<_T>(operand);
}

/*!
 * @brief Using the std::optional type
 */
template <typename _Tp>
using optional = std::optional<_Tp>;

/*!
 * @brief Using the std::variant type
 */
template <typename... _Tp>
using variant = std::variant<_Tp...>;

/*!
 * @brief Using the std::filesystem types
 */
namespace filesystem = std::filesystem;

} /* End namespace common */
} /* End namespace tmx */

#endif /* TMX_PLATFORM_INCLUDE */
