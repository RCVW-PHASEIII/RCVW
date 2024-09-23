/*
 * Copyright (c) 2022 Battelle Memorial Institute
 *
 * All Rights Reserved.
 *
 * @file tmx_version.hpp
 *
 * This header defines a versioning structure
 *
 * One must at least #define TMX_VERSION with a semantic
 * version number, i.e. maj.min.patch, prior to including
 * this header. Or, you can individually define the major,
 * minor and patch revision numbers.
 *
 * Because there are no default version values, this header will
 * necessarily cause a compiler error if the key pre-processor
 * defines are missing. Either the entire TMX_VERSION must be
 * set, or TMX_MAJOR_VERSION, TMX_MINOR_VERSION, and
 * TMX_PATCH_VERSION must all be set.
 *
 * Additionally, since pre-processor macros are used to
 * define the version number, it is expected these be passed
 * in during compile time and therefore fixed for any component
 * compiled with those values. However, it is not out of the
 * question that an individual module may use its own version,
 * which is why there is a templated structure. The revision numbers
 * should be literal values so the version structure is a complete
 * type a compile-time.
 *
 *  Created on: Oct 6, 2022
 *      @author: Gregory M. Baumgardner
 */

#ifndef PLATFORM_INCLUDE_TMX_COMMON_PLATFORM_MODULES_TMX_VERSION_HPP
#define PLATFORM_INCLUDE_TMX_COMMON_PLATFORM_MODULES_TMX_VERSION_HPP

#include <tmx/common/platform/types/bytes.hpp>
#include <tmx/common/platform/types/arrays.hpp>

#include <cstddef>
#include <type_traits>

#ifndef TMX_VERSION_SEPERATOR
#define TMX_VERSION_SEPARATOR .
#endif

namespace tmx {
namespace common {

template <std::size_t _Major, std::size_t _Minor, std::size_t _Patch>
struct tmx_version_t: public std::true_type {
	typedef tmx_version_t<_Major, _Minor, _Patch> self_type;

	static constexpr std::size_t major = _Major;
	static constexpr std::size_t minor = _Minor;
	static constexpr std::size_t patch = _Patch;

private:
	static constexpr auto _major = make_string<major>();
	static constexpr auto _minor = make_string<minor>();
	static constexpr auto _patch = make_string<patch>();
	static constexpr auto _sep_str = TMX_CPP_QUOTE(TMX_VERSION_SEPARATOR);
	static constexpr auto _sep = TMX_STATIC_STRING_VAR(_sep_str);

	static constexpr auto _version =
			concat(_major, concat(_sep,
			concat(_minor, concat(_sep, _patch))));

public:
	static constexpr const_string version { _version.c_str() };
};

} /* End namespace common */
} /* End namespace tmx */

#endif /* PLATFORM_INCLUDE_TMX_COMMON_PLATFORM_MODULES_TMX_VERSION_HPP */

#ifndef TMX_VERSION_DEFINED

namespace tmx {
namespace common {

#if defined(TMX_VERSION)
struct _ver_split {
	static constexpr auto version_str = TMX_CPP_QUOTE(TMX_VERSION);
	static constexpr const_string version { version_str };

	static constexpr auto _sep = TMX_CPP_QUOTE(TMX_VERSION_SEPARATOR);
	static constexpr auto _first = version.find_first_of(_sep);
	static constexpr auto _last = version.find_last_of(_sep);

	static_assert(_first != const_string::npos && _last != const_string::npos,
			"TMX_VERSION must follow Major.Minor.Patch semantic numbering. A different separator may be set using TMX_VERSION_SEPARATOR.");

	static constexpr auto _major_s = version_str;
	static constexpr auto _major = digitize(make_string<&_major_s, _first>());

	static constexpr auto _minor_s = version_str + _first + 1;
	static constexpr auto _minor = digitize(make_string<&_minor_s, _last - _first - 1>());

	static constexpr auto _patch_s = version_str + _last + 1;
	static constexpr auto _patch = digitize(make_string<&_patch_s>());

	static constexpr std::size_t major = dedigitize<10>(_major);
	static constexpr std::size_t minor = dedigitize<10>(_minor);
	static constexpr std::size_t patch = dedigitize<10>(_patch);
};

using tmx_version = tmx_version_t< _ver_split::major, _ver_split::minor, _ver_split::patch >;
#define TMX_VERSION_DEFINED

#elif defined(TMX_MAJOR_VERSION)
static_assert(TMX_MAJOR_VERSION > 0, "Could not determine software major revision. Please set TMX_VERSION or TMX_MAJOR_VERSION");

#ifndef TMX_MINOR_VERSION
static_assert(false, "Could not determine software minor revision. Please set TMX_VERSION or TMX_MINOR_VERSION");

// This is solely for IDE purposes
#define TMX_MINOR_VERSION 0
#endif

#ifndef TMX_PATCH_VERSION
static_assert(false, "Could not determine software patch revision. Please set TMX_VERSION or TMX_PATCH_VERSION");

// This is solely for IDE purposes
#define TMX_PATCH_VERSION 0
#endif

using tmx_version = tmx_version_t<TMX_MAJOR_VERSION, TMX_MINOR_VERSION, TMX_PATCH_VERSION>;
#define TMX_VERSION_DEFINED

#endif

} /* End namespace common */
} /* End namespace tmx */

#endif /* TMX_VERSION_DEFINED */
