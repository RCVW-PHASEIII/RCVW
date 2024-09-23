/*
 * Copyright (c) 2022 Battelle Memorial Institute
 *
 * All Rights Reserved.
 *
 * @file tmx_module.hpp
 *
 * This header defines a module structure
 *
 * Every module is indicated by a tag, which is nothing more
 * than a type. The module tag is used to obtain the module
 * name and the version number. There are defaults for
 * everything, so that there will always be a module definition
 * for a tag.
 *
 * This capability is intended to be used with some inventory
 * system that is tracking modules in every running version of
 * TMX. Note that the platform only defines the module definition
 * and does no tracking of its own.
 *
 *  Created on: Oct 6, 2022
 *      @author: Gregory M. Baumgardner
 */

#ifndef PLATFORM_INCLUDE_TMX_COMMON_PLATFORM_MODULES_TMX_MODULE_HPP_
#define PLATFORM_INCLUDE_TMX_COMMON_PLATFORM_MODULES_TMX_MODULE_HPP_

#include <tmx/common/platform/types/introspect.hpp>

#include <type_traits>

namespace tmx {
namespace common {

template <typename _Tag>
struct tmx_module_version {
	struct no_version: public std::false_type {
		static constexpr int major = -1;
		static constexpr int minor = -1;
		static constexpr int patch = -1;

		static constexpr const char *version = "Unknown";
	};

	using version =
#ifdef TMX_VERSION_DEFINED
		tmx_version;
#else
		no_version;
#endif
};

template <typename _Tag>
struct tmx_module {
	static inline constexpr const_string get_name() {
		return type_short_name<_Tag>();
	}

	static inline constexpr auto get_version() {
		typedef typename tmx_module_version<_Tag>::version version;
		return version {};
	}
};

} /* End namespace common */
} /* End namespace tmx */

#endif /* PLATFORM_INCLUDE_TMX_COMMON_PLATFORM_MODULES_TMX_MODULE_HPP_ */
