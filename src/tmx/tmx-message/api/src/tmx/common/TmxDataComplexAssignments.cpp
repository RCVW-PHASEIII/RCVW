/*!
 * Copyright (c) 2021 Battelle Memorial Institute
 *
 * All Rights Reserved.
 *
 * @file TmxDataComplexAssignments.cpp
 *
 *  Created on: Jul 29, 2021
 *      @author: gmb
 */

#include <tmx/common/TmxData.hpp>
#include <tmx/common/TmxVariant.hpp>

#include <utility>

using namespace tmx::common::types;

namespace tmx {
namespace common {

TmxData &TmxData::operator =(string8 &&value) {
	if (this->_scalarValue)
		this->_scalarValue->emplace<string8>(std::forward<string8>(value));
	else
		this->_scalarValue.reset(new TmxTypeVariant(std::forward<string8>(value)));
	this->_arrayValue.reset();
	this->_mapValue.reset();
	this->_contents = typeid(string8);
	return *this;
}

TmxData &TmxData::operator =(const string8 &value) {
	return this->operator =(string8(value));
}

TmxData &TmxData::operator =(typename string8::type value) {
	return this->operator =((typename string8::view_type) value);
}

TmxData &TmxData::operator =(typename string8::view_type value) {
	return this->operator =(string8(value));
}

TmxData &TmxData::operator =(const typename string8::char_t *value) {
	return this->operator =(typename string8::view_type(value));
}

TmxData &TmxData::operator =(string16 &&value) {
	if (this->_scalarValue)
		this->_scalarValue->emplace<string16>(std::forward<string16>(value));
	else
		this->_scalarValue.reset(new TmxTypeVariant(std::forward<string16>(value)));
	this->_arrayValue.reset();
	this->_mapValue.reset();
	this->_contents = typeid(string16);
	return *this;
}

TmxData &TmxData::operator =(const string16 &value) {
	return this->operator =(string16(value));
}

TmxData &TmxData::operator =(typename string16::type value) {
	return this->operator =((typename string16::view_type) value);
}

TmxData &TmxData::operator =(typename string16::view_type value) {
	return this->operator =(string16(value));
}

TmxData &TmxData::operator =(const typename string16::char_t *value) {
	return this->operator =(typename string16::view_type(value));
}

TmxData &TmxData::operator =(string32 &&value) {
	if (this->_scalarValue)
		this->_scalarValue->emplace<string32>(std::forward<string32>(value));
	else
		this->_scalarValue.reset(new TmxTypeVariant(std::forward<string32>(value)));
	this->_arrayValue.reset();
	this->_mapValue.reset();
	this->_contents = typeid(string32);
	return *this;
}

TmxData &TmxData::operator =(const string32 &value) {
	return this->operator =(string32(value));
}

TmxData &TmxData::operator =(typename string32::type value) {
	return this->operator =((typename string32::view_type) value);
}

TmxData &TmxData::operator =(typename string32::view_type value) {
	return this->operator =(string32(value));
}

TmxData &TmxData::operator =(const typename string32::char_t *value) {
	return this->operator =(typename string32::view_type(value));
}

TmxData &TmxData::operator =(wstring &&value) {
	if (this->_scalarValue)
		this->_scalarValue->emplace<wstring>(std::forward<wstring>(value));
	else
		this->_scalarValue.reset(new TmxTypeVariant(std::forward<wstring>(value)));
	this->_arrayValue.reset();
	this->_mapValue.reset();
	this->_contents = typeid(wstring);
	return *this;
}

TmxData &TmxData::operator =(const wstring &value) {
	return this->operator =(wstring(value));
}

TmxData &TmxData::operator =(typename wstring::type value) {
	return this->operator =((typename wstring::view_type) value);
}

TmxData &TmxData::operator =(typename wstring::view_type value) {
	return this->operator =(wstring(value));
}

TmxData &TmxData::operator =(const typename wstring::char_t *value) {
	return this->operator =(typename wstring::view_type(value));
}

TmxData &TmxData::operator =(array_type<TmxData> &&value) {
	this->_arrayValue.reset(new array_type<TmxData>(value));
	this->_scalarValue.reset();
	this->_mapValue.reset();
	this->_contents = typeid(array_type<TmxData>);
	return *this;
}

TmxData &TmxData::operator =(const array_type<TmxData> &value) {
	return this->operator =(array_type<TmxData>(value));
}

TmxData &TmxData::operator =(map_type<string8, TmxData> &&value) {
	this->_mapValue.reset(new map_type<string8, TmxData>(value));
	this->_scalarValue.reset();
	this->_arrayValue.reset();
	this->_contents = typeid(map_type<string8, TmxData>);
	return *this;
}

TmxData &TmxData::operator =(const map_type<string8, TmxData> &value) {
	return this->operator =(map_type<string8, TmxData>(value));
}

} /* End namespace common */
} /* End namespace tmx */

