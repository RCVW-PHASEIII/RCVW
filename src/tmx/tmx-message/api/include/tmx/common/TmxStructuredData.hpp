/*!
 * Copyright (c) 2021 Battelle Memorial Institute
 *
 * All Rights Reserved.
 *
 * @file TmxStructuredData.hpp
 *
 *  Created on: Aug 19, 2021
 *      @author: gmb
 */

#ifndef API_INCLUDE_TMX_COMMON_TMXSTRUCTUREDDATA_HPP_
#define API_INCLUDE_TMX_COMMON_TMXSTRUCTUREDDATA_HPP_

#include <tmx/common/TmxData.hpp>

#include <tmx/common/types/enum_type.hpp>
#include <tmx/common/types/string_type.hpp>

namespace tmx {
namespace common {

template <typename _E>
class TmxStructuredData_EnumeratedNames: public TmxData {
public:
	typedef types::enum_type<_E> enum_t;
	typedef typename enum_t::Enum Enum;
	typedef typename enum_t::type type;

	using TmxData::operator [];
	using TmxData::operator =;
	using TmxData::operator typename types::null_type::type;
	using TmxData::operator typename types::boolean_type::type;
	using TmxData::operator typename types::int8::type;
	using TmxData::operator typename types::int16::type;
	using TmxData::operator typename types::int32::type;
	using TmxData::operator typename types::int64::type;
	using TmxData::operator typename types::uint8::type;
	using TmxData::operator typename types::uint16::type;
	using TmxData::operator typename types::uint32::type;
	using TmxData::operator typename types::uint64::type;
	using TmxData::operator typename types::float32::type;
	using TmxData::operator typename types::float64::type;
	using TmxData::operator typename types::float128::type;
	using TmxData::operator typename types::string8::view_type;
	using TmxData::operator typename types::string8::view_type::const_pointer;
	using TmxData::to_array;
	using TmxData::to_float;
	using TmxData::to_int;
	using TmxData::to_map;
	using TmxData::to_string;
	using TmxData::to_uint;

	explicit TmxStructuredData_EnumeratedNames<_E>() {
		this->initialize();
	}

	TmxStructuredData_EnumeratedNames<_E>(const TmxData &copy):
			TmxData(copy) { }

	TmxData &operator [](Enum field) {
		return this->operator [](enum_t::get_name(field).data());
	}

	TmxData &operator [](const enum_t &field) {
		return this->operator []((Enum) field);
	}

	TmxData &operator [](type field) {
		return this->operator [](enum_t::get_value(field));
	}

private:
	void initialize() {
		for (auto name: enum_t::get_names())
			this->operator [](name);
	}
};

} /* End namespace common */
} /* End namespace tmx */

#endif /* API_INCLUDE_TMX_COMMON_TMXSTRUCTUREDDATA_HPP_ */
