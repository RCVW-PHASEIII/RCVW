/*!
 * Copyright (c) 2021 Battelle Memorial Institute
 *
 * All Rights Reserved.
 *
 * @file TmxDataPrimitiveAssignments.cpp
 *
 *  Created on: Jul 21, 2021
 *      @author: gmb
 */

#include <tmx/common/TmxData.hpp>

#include <tmx/common/TmxVariant.hpp>

#include <utility>

using namespace tmx::common::types;

namespace tmx {
namespace common {

TmxData &TmxData::operator =(TmxData &&copy) {
	this->_scalarValue.reset();
	this->_arrayValue.reset();
	this->_mapValue.reset();

	if (copy._scalarValue)
		this->_scalarValue.reset(new TmxTypeVariant(*(copy._scalarValue)));
	else if (copy._arrayValue)
		this->_arrayValue.reset(new array_type<TmxData>(*(copy._arrayValue)));
	else if (copy._mapValue)
		this->_mapValue.reset(new map_type<string8, TmxData>(*(copy._mapValue)));

	this->_contents = copy._contents;

	copy.clear();
	return *this;
}

TmxData &TmxData::operator =(const TmxData &copy) {
	this->_scalarValue.reset();
	this->_arrayValue.reset();
	this->_mapValue.reset();

	if (copy._scalarValue)
		this->_scalarValue.reset(new TmxTypeVariant(*(copy._scalarValue)));
	else if (copy._arrayValue)
		this->_arrayValue.reset(new array_type<TmxData>(*(copy._arrayValue)));
	else if (copy._mapValue)
		this->_mapValue.reset(new map_type<string8, TmxData>(*(copy._mapValue)));

	this->_contents = copy._contents;
	return *this;
}

TmxData &TmxData::operator =(typename null_type::type &&value) {
	return this->operator =(null_type(value));
}

TmxData &TmxData::operator =(null_type &&value) {
	if (this->_scalarValue)
		this->_scalarValue->emplace<null_type>(std::forward<null_type>(value));
	else
		this->_scalarValue.reset(new TmxTypeVariant(std::forward<null_type>(value)));
	this->_arrayValue.reset();
	this->_mapValue.reset();
	this->_contents = typeid(null_type);
	return *this;
}

TmxData &TmxData::operator =(const null_type &value) {
	return this->operator =(std::move(value));
}

TmxData &TmxData::operator =(typename boolean_type::type &&value) {
	return this->operator =(boolean_type(value));
}

TmxData &TmxData::operator =(boolean_type &&value) {
	if (this->_scalarValue)
		this->_scalarValue->emplace<boolean_type>(std::forward<boolean_type>(value));
	else
		this->_scalarValue.reset(new TmxTypeVariant(std::forward<boolean_type>(value)));
	this->_arrayValue.reset();
	this->_mapValue.reset();
	this->_contents = typeid(boolean_type);
	return *this;
}

TmxData &TmxData::operator =(const boolean_type &value) {
	return this->operator =(std::move(value));
}

TmxData &TmxData::operator =(typename int8::type value) {
	if (this->_scalarValue)
		this->_scalarValue->emplace<int8>(std::forward<int8>(value));
	else
		this->_scalarValue.reset(new TmxTypeVariant(int8(value)));
	this->_arrayValue.reset();
	this->_mapValue.reset();
	this->_contents = typeid(int8);
	return *this;
}

TmxData &TmxData::operator =(typename int16::type value) {
	if (this->_scalarValue)
		this->_scalarValue->emplace<int16>(std::forward<int16>(value));
	else
		this->_scalarValue.reset(new TmxTypeVariant(int16(value)));
	this->_arrayValue.reset();
	this->_mapValue.reset();
	this->_contents = typeid(int16);
	return *this;
}

TmxData &TmxData::operator =(typename int32::type value) {
	this->_scalarValue.reset(new TmxTypeVariant(int32(value)));
	this->_arrayValue.reset();
	this->_mapValue.reset();
	this->_contents = typeid(int32);
	return *this;
}

TmxData &TmxData::operator =(typename int64::type value) {
	if (this->_scalarValue)
		this->_scalarValue->emplace<int64>(std::forward<int64>(value));
	else
		this->_scalarValue.reset(new TmxTypeVariant(int64(value)));
	this->_arrayValue.reset();
	this->_mapValue.reset();
	this->_contents = typeid(int64);
	return *this;
}

TmxData &TmxData::operator =(typename uint8::type value) {
	if (this->_scalarValue)
		this->_scalarValue->emplace<uint8>(std::forward<uint8>(value));
	else
		this->_scalarValue.reset(new TmxTypeVariant(uint8(value)));
	this->_arrayValue.reset();
	this->_mapValue.reset();
	this->_contents = typeid(uint8);
	return *this;
}

TmxData &TmxData::operator =(typename uint16::type value) {
	if (this->_scalarValue)
		this->_scalarValue->emplace<uint16>(std::forward<uint16>(value));
	else
		this->_scalarValue.reset(new TmxTypeVariant(uint16(value)));
	this->_arrayValue.reset();
	this->_mapValue.reset();
	this->_contents = typeid(uint16);
	return *this;
}

TmxData &TmxData::operator =(typename uint32::type value) {
	if (this->_scalarValue)
		this->_scalarValue->emplace<uint32>(std::forward<uint32>(value));
	else
		this->_scalarValue.reset(new TmxTypeVariant(uint32(value)));
	this->_arrayValue.reset();
	this->_mapValue.reset();
	this->_contents = typeid(uint32);
	return *this;
}

TmxData &TmxData::operator =(typename uint64::type value) {
	if (this->_scalarValue)
		this->_scalarValue->emplace<uint64>(std::forward<uint64>(value));
	else
		this->_scalarValue.reset(new TmxTypeVariant(uint64(value)));
	this->_arrayValue.reset();
	this->_mapValue.reset();
	this->_contents = typeid(uint64);
	return *this;
}

TmxData &TmxData::operator =(typename float32::type value) {
	if (this->_scalarValue)
		this->_scalarValue->emplace<float32>(std::forward<float32>(value));
	else
		this->_scalarValue.reset(new TmxTypeVariant(float32(value)));
	this->_arrayValue.reset();
	this->_mapValue.reset();
	this->_contents = typeid(float32);
	return *this;
}

TmxData &TmxData::operator =(typename float64::type value) {
	if (this->_scalarValue)
		this->_scalarValue->emplace<float64>(std::forward<float64>(value));
	else
		this->_scalarValue.reset(new TmxTypeVariant(float64(value)));
	this->_arrayValue.reset();
	this->_mapValue.reset();
	this->_contents = typeid(float64);
	return *this;
}

#ifdef TMX_FLOAT128
TmxData &TmxData::operator =(typename float128::type value) {
	if (this->_scalarValue)
		this->_scalarValue->emplace<float128>(std::forward<float128>(value));
	else
		this->_scalarValue.reset(new TmxTypeVariant(float128(value)));
	this->_arrayValue.reset();
	this->_mapValue.reset();
	this->_contents = typeid(float128);
	return *this;
}
#endif

} /* End namespace common */
} /* End namespace tmx */

