/*!
 * Copyright (c) 2021 Battelle Memorial Institute
 *
 * All Rights Reserved.
 *
 * @file TmxData.cpp
 *
 *  Created on: Jul 20, 2021
 *      @author: gmb
 */

#include <tmx/common/TmxData.hpp>
#include <tmx/common/TmxTypeRegistry.hpp>
#include <tmx/common/TmxVariant.hpp>

using namespace tmx::common::types;

namespace tmx {
namespace common {

static auto &reg = TmxTypeRegistry::get_types();

TmxData::TmxData():	_contents(typeid(null_type)) {
	// Force a scalar assignment
	this->operator =(null_type());
}

TmxData::TmxData(TmxData &&copy): TmxData() {
	// Save a copy
	this->operator =(std::forward<TmxData>(copy));
}

TmxData::TmxData(const TmxData &copy): TmxData() {
	// Save a copy
	this->operator =(copy);
}

TmxData::~TmxData() {
	// Memory cleaned up automatically
}

void TmxData::clear() {
	// The empty state is a null value
	this->operator =(null_type());
}

bool TmxData::empty() const {
	return this->contains<null_type>();
}

const std::type_info &TmxData::get_type() const {
	return reg.get(this->_contents).type();
}

bool TmxData::contains(typename string::view_type name, typename string::view_type nmspace) const {
	auto &_descriptor = reg.get(name, nmspace);
	return _descriptor == reg.get(this->get_type());
}

bool TmxData::contains(const typename string::char_t *name, const typename string::char_t *nmspace) const {
	return this->contains(typename string::view_type(name), typename string::view_type(nmspace));
}

typename string::view_type TmxData::contents() const {
	if (this->_arrayValue)
		return types::support::type_id_name(*(this->_arrayValue));
	else if (this->_mapValue)
		return types::support::type_id_name(*(this->_mapValue));

	return reg.get(this->get_type()).get_type_id();

}

std::ostream &operator<<(std::ostream &os, const any &data) {
	// Use JSON encoding for map and array types
	if (data.contains< types::map_type<types::string8, any> >() ||
			data.contains< types::array_type<any> >())
		os << data.to_string("json");
	else
		os << data.to_string();

	return os;
}

std::istream &operator>>(std::istream &is, any &data) {
	// TODO: Consume every line?
	// Read in the string contents
	string s;
	is >> s;

	data = s;
	return is;
}

} /* namespace common */
} /* namespace tmx */
