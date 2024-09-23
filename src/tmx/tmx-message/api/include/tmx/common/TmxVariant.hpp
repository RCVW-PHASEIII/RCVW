/*!
 * Copyright (c) 2021 Battelle Memorial Institute
 *
 * All Rights Reserved.
 *
 * @file TmxVariant.hpp
 *
 *  Created on: Aug 14, 2021
 *      @author: gmb
 */

#ifndef API_SRC_TMX_COMMON_SUPPORT_TMXVARIANT_HPP_
#define API_SRC_TMX_COMMON_SUPPORT_TMXVARIANT_HPP_

#include <tmx/common/types/array_type.hpp>
#include <tmx/common/types/boolean_type.hpp>
#include <tmx/common/types/bytes_type.hpp>
#include <tmx/common/types/enum_type.hpp>
#include <tmx/common/types/float_type.hpp>
#include <tmx/common/types/int_type.hpp>
#include <tmx/common/types/map_type.hpp>
#include <tmx/common/types/null_type.hpp>
#include <tmx/common/types/record_type.hpp>
#include <tmx/common/types/string_type.hpp>

#include <variant>

namespace tmx {
namespace common {

template <class... _Types>
class TmxVariant: public std::variant<_Types...> {
public:
	typedef std::variant<_Types...> type;

	template <typename _T>
	TmxVariant(_T &&v): std::variant<_Types...>(std::forward<_T>(v)) { }

	typedef tmx::common::types::record_type<_Types...> all_types;

	static constexpr const auto count = all_types::count;

	using std::variant<_Types...>::operator =;

	using std::variant<_Types...>::emplace;
	using std::variant<_Types...>::index;
};

// Using only the basic types since the
using TmxTypeVariantType = TmxVariant<
		types::null_type,
		types::boolean_type,
		types::int2,
		types::int3,
		types::int4,
		types::int5,
		types::int6,
		types::int7,
		types::int8,
		types::int9,
		types::int10,
		types::int11,
		types::int12,
		types::int13,
		types::int14,
		types::int15,
		types::int16,
		types::int17,
		types::int18,
		types::int19,
		types::int20,
		types::int21,
		types::int22,
		types::int23,
		types::int24,
		types::int25,
		types::int26,
		types::int27,
		types::int28,
		types::int29,
		types::int30,
		types::int31,
		types::int32,
		types::int33,
		types::int34,
		types::int35,
		types::int36,
		types::int37,
		types::int38,
		types::int39,
		types::int40,
		types::int41,
		types::int42,
		types::int43,
		types::int44,
		types::int45,
		types::int46,
		types::int47,
		types::int48,
		types::int49,
		types::int50,
		types::int51,
		types::int52,
		types::int53,
		types::int54,
		types::int55,
		types::int56,
		types::int57,
		types::int58,
		types::int59,
		types::int60,
		types::int61,
		types::int62,
		types::int63,
		types::int64,
		types::uint1,
		types::uint2,
		types::uint3,
		types::uint4,
		types::uint5,
		types::uint6,
		types::uint7,
		types::uint8,
		types::uint9,
		types::uint10,
		types::uint11,
		types::uint12,
		types::uint13,
		types::uint14,
		types::uint15,
		types::uint16,
		types::uint17,
		types::uint18,
		types::uint19,
		types::uint20,
		types::uint21,
		types::uint22,
		types::uint23,
		types::uint24,
		types::uint25,
		types::uint26,
		types::uint27,
		types::uint28,
		types::uint29,
		types::uint30,
		types::uint31,
		types::uint32,
		types::uint33,
		types::uint34,
		types::uint35,
		types::uint36,
		types::uint37,
		types::uint38,
		types::uint39,
		types::uint40,
		types::uint41,
		types::uint42,
		types::uint43,
		types::uint44,
		types::uint45,
		types::uint46,
		types::uint47,
		types::uint48,
		types::uint49,
		types::uint50,
		types::uint51,
		types::uint52,
		types::uint53,
		types::uint54,
		types::uint55,
		types::uint56,
		types::uint57,
		types::uint58,
		types::uint59,
		types::uint60,
		types::uint61,
		types::uint62,
		types::uint63,
		types::uint64,
		types::float32,
		types::float64,
#ifdef TMX_FLOAT128
#ifndef _MSC_VER
		types::float128,
#endif
#endif
		types::string8,
		types::string16,
		types::string32,
		types::wstring>;

class TmxTypeVariant: public TmxTypeVariantType {
public:
	template <typename _T>
	TmxTypeVariant(_T &&v): TmxTypeVariantType(std::forward<_T>(v)) { }

	using TmxTypeVariantType::operator =;
	using TmxTypeVariantType::emplace;
	using TmxTypeVariantType::index;
};

} /* End namespace common */
} /* End namespace tmx */


#endif /* API_SRC_TMX_COMMON_SUPPORT_TMXVARIANT_HPP_ */
