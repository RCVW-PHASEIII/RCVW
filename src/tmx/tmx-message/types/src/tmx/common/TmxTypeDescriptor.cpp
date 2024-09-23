/*!
 * Copyright (c) 2021 Battelle Memorial Institute
 *
 * All Rights Reserved.
 *
 * @file TmxTypeDescriptor.cpp
 *
 *  Created on: Aug 6, 2021
 *      @author: gmb
 */

#include <tmx/common/TmxTypeDescriptor.hpp>

#include <iostream>
#include <typeindex>

namespace tmx {
namespace common {

TmxTypeDescriptor::TmxTypeDescriptor(std::shared_ptr<const void> instance,
					std::type_info const &id, std::string name) noexcept:
		_instance(instance), _id(id), _path(name) { }

TmxTypeDescriptor::TmxTypeDescriptor(TmxTypeDescriptor const &copy) noexcept:
		_instance(copy._instance), _id(copy._id), _path(copy._path) { }

std::shared_ptr<const void> const TmxTypeDescriptor::get_instance() const noexcept {
	return this->_instance;
}

std::type_info const &TmxTypeDescriptor::get_typeid() const noexcept {
	return this->_id;
}

filesystem::path const &TmxTypeDescriptor::get_path() const noexcept {
    return this->_path;
}

std::string TmxTypeDescriptor::get_type_name() const noexcept {
    return this->_path.string();
}

std::string TmxTypeDescriptor::get_type_short_name() const noexcept {
    return this->_path.filename().string();
}

std::string TmxTypeDescriptor::get_type_namespace() const noexcept {
    return this->_path.parent_path().string();
}

bool TmxTypeDescriptor::operator ==(std::type_info const &other) const noexcept {
	return std::type_index(this->get_typeid()) == other;
}

bool TmxTypeDescriptor::operator ==(const TmxTypeDescriptor &other) const noexcept {
	return this->operator==(other.get_typeid());
}

bool TmxTypeDescriptor::operator !=(std::type_info const &other) const noexcept {
	return std::type_index(this->get_typeid()) != other;
}

bool TmxTypeDescriptor::operator !=(const TmxTypeDescriptor &other) const noexcept {
	return this->operator!=(other.get_typeid());
}

TmxTypeDescriptor::operator bool() const noexcept {
	return this->_id != typeid(void) && !this->_path.empty();
}

} /* End namespace common */
} /* End namespace tmx */
