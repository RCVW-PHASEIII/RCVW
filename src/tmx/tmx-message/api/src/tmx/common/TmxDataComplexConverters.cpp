/*!
 * Copyright (c) 2021 Battelle Memorial Institute
 *
 * All Rights Reserved.
 *
 * @file TmxDataComplexConverters.cpp
 *
 *  Created on: Jul 21, 2021
 *      @author: gmb
 */

#include <tmx/common/TmxData.hpp>

#include <tmx/common/TmxTypeVisitor.hpp>
#include <tmx/message/codec/TmxCodec.hpp>

#include <sstream>

using namespace tmx::common::types;
using namespace tmx::message::codec;

namespace tmx {
namespace common {

TmxData::operator typename string::view_type() const {
	typedef typename string::view_type _t;
	return (_t)this->to_string();
}

TmxData::operator const typename string::char_t *() const {
	return this->to_string().c_str();
}

string TmxData::to_string(typename string::view_type encoding) const {
	if (encoding.empty())
		encoding = "string";

	auto msg = TmxCodec::encode(*this, encoding.data());
	if (!msg)
		return string("");

	host_bytes_type<typename string::char_t> bytes { msg->get_payload().data(), msg->get_payload().length() };
	return string(bytes.data(), bytes.length());
}

array_type<TmxData> TmxData::to_array() const {
	typedef array_type<TmxData> _at;

	// If this is already an array, return it
	if (this->_arrayValue)
		return array_type<TmxData>(*(this->_arrayValue));

	// Otherwise, make it an array with existing contents
	if (this->empty())
		return _at { };
	else
		return _at { TmxData { *this } };
}

map_type<string8, TmxData> TmxData::to_map() const {
	typedef map_type<string8, TmxData> _mt;

	// If this is already a map, return it
	if (this->_mapValue)
		return map_type<string8, TmxData>(*(this->_mapValue));

	// Otherwise, make it a map with existing contents
	typename _mt::type theMap;
	if (!this->empty())
		theMap[string8("value")] = TmxData(*this);

	return _mt(theMap);
}

} /* namespace common */
} /* namespace tmx */
