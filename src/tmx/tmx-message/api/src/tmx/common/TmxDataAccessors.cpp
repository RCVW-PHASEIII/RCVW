/*!
 * Copyright (c) 2021 Battelle Memorial Institute
 *
 * All Rights Reserved.
 *
 * @file TmxDataAccessors.cpp
 *
 *  Created on: Jul 21, 2021
 *      @author: gmb
 */

#include <tmx/common/TmxData.hpp>
#include <tmx/common/TmxLogger.hpp>
#include <tmx/common/TmxVariant.hpp>

using namespace tmx::common::types;

namespace tmx {
namespace common {

static TmxData _null_data;

TmxData &TmxData::operator [](const string &key) {
	typedef map_type<string8, TmxData> _mt;

	// Convert to a map if necessary
	if (!this->_mapValue)
		this->operator =(this->to_map());

	auto &m = *(this->_mapValue);
	return m[key];
}

TmxData &TmxData::operator [](const string &key) const {
	// Cannot convert to a map, so print out a warning
	// and return a null type
	if (!this->_mapValue) {
		TLOG(debug2) << "Map access for " << key << " requested on "
				<< (this->_arrayValue ? "array" : "scalar")
				<< " data: Returning null.";
		return _null_data;
	}

	auto &m = *(this->_mapValue);
	if (!m.count(key)) {
		TLOG(debug2) << "Missing map key: " << key
				<< ": Returning null";

		return _null_data;
	}

	return m[key];
}

TmxData &TmxData::operator [](const typename string::type &key) {
	return this->operator [](string(key));
}

TmxData &TmxData::operator [](const typename string::type &key) const {
	return this->operator [](string(key));
}

TmxData &TmxData::operator [](typename string::view_type key) {
	return this->operator [](string(key));
}

TmxData &TmxData::operator [](typename string::view_type key) const {
	return this->operator [](string(key));
}

TmxData &TmxData::operator [](typename string::const_pointer key) {
	return this->operator [](string(key));
}

TmxData &TmxData::operator [](typename string::const_pointer key) const {
	return this->operator [](string(key));
}

TmxData &TmxData::operator [](typename int16::type position) {
	return this->operator []((typename uint64::type) position);
}

TmxData &TmxData::operator [](typename int16::type position) const {
	return this->operator []((typename uint64::type) position);
}

TmxData &TmxData::operator [](typename int32::type position) {
	return this->operator []((typename uint64::type) position);
}

TmxData &TmxData::operator [](typename int32::type position) const {
	return this->operator []((typename uint64::type) position);
}

TmxData &TmxData::operator [](typename int64::type position) {
	return this->operator []((typename uint64::type) position);
}

TmxData &TmxData::operator [](typename int64::type position) const {
	return this->operator []((typename uint64::type) position);
}

TmxData &TmxData::operator [](typename uint16::type position) {
	return this->operator []((typename uint64::type) position);
}

TmxData &TmxData::operator [](typename uint16::type position) const {
	return this->operator []((typename uint64::type) position);
}

TmxData &TmxData::operator [](typename uint32::type position) {
	return this->operator []((typename uint64::type) position);
}

TmxData &TmxData::operator [](typename uint32::type position) const {
	return this->operator []((typename uint64::type) position);
}

TmxData &TmxData::operator [](typename uint64::type position) {
	typedef array_type<TmxData> _at;
	typedef map_type<string8, TmxData> _mt;

	if (!this->_arrayValue && !this->_mapValue)
		this->operator =(this->to_array());

	if (this->_mapValue) {
		auto key = string(std::to_string(position));
		_mt &m = *(this->_mapValue);
		return m[key];
	} else {
		_at &a = *(this->_arrayValue);

		if (a.size() <= position)
			a.resize(position + 1, _null_data);

		return a.at(position);
	}
}

TmxData &TmxData::operator [](typename uint64::type position) const {
	if (!this->_arrayValue) {
		if (this->_mapValue)
			return this->operator [](std::to_string(position).c_str());

		TLOG(debug2) << "Array access requested on scalar data"
				<< ": Returning null.";
		return _null_data;
	}

	auto &a = *(this->_arrayValue);
	if (position >= a.size()) {
		TLOG(debug2) << "Array index out-of-bounds: " << position
				<< ": Returning null.";
		return _null_data;
	}

	return a[position];
}

} /* End namespace common */
} /* End namespace tmx */
