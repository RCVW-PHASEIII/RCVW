/*!
 * Copyright (c) 2021 Battelle Memorial Institute
 *
 * All Rights Reserved.
 *
 * @file type_registry.hpp
 *
 *  Created on: Jul 18, 2021
 *      @author: gmb
 */

#ifndef API_INCLUDE_TMX_COMMON_TYPES_SUPPORT_TYPE_REGISTRY_HPP_
#define API_INCLUDE_TMX_COMMON_TYPES_SUPPORT_TYPE_REGISTRY_HPP_

#include <tmx/common/types/enum_type.hpp>

#include <array>
#include <string>
#include <string_view>
#include <map>

namespace tmx {
namespace common {
namespace types {
namespace support {

template <typename _E, typename... _Elements>
struct _type_registry_t {
	typedef _type_registry_t<_E, _Elements...> self_type;
	typedef tmx::common::types::enum_type<_E> enum_type;
	typedef std::tuple<_Elements...> tuple_type;

	static constexpr auto count = sizeof...(_Elements);
	static_assert(enum_type::count == count, "the number of types must equal the number of enum values");

	template <_E Val>
	static inline auto allocator() {
		typedef typename std::tuple_element<*index(Val), tuple_type>::type type;
		static std::allocator<type> _allocator;
		return _allocator;
	}

	static inline constexpr auto types() {
		return _types;
	}

private:
	static constexpr tuple_type _types { };

};

} /* End namespace support */
} /* End namespace types */
} /* End namespace common */
} /* End namespace tmx */


#endif /* API_INCLUDE_TMX_COMMON_TYPES_SUPPORT_TYPE_REGISTRY_HPP_ */
