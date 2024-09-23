/*!
 * Copyright (c) 2021 Battelle Memorial Institute
 *
 * All Rights Reserved.
 *
 * @file type_descriptor.hpp
 *
 *  Created on: Jun 15, 2021
 *      @author: Gregory M. Baumgardner
 */

#ifndef INCLUDE_TMX_COMMON_TYPES_TYPE_DESCRIPTOR_HPP_
#define INCLUDE_TMX_COMMON_TYPES_TYPE_DESCRIPTOR_HPP_

#include <tmx/common/types/introspect.hpp>

#include <memory>
#include <string_view>
#include <typeindex>

namespace tmx {
namespace common {
namespace types {

/*!
 * This structure holds information about available types. The intention is to
 * be able to store the descriptor of the type in some container, and access it
 * based solely on its identifier, or its namespace and type name.
 */
struct type_descriptor {
	typedef type_descriptor self_type;

	/*!
	 * No default constructor
	 */
	type_descriptor() = delete;

	/*!
	 * Copy constructor
	 */
	explicit type_descriptor(const self_type &other):
		index(other.index),
		type_id(other.type_id),
		name_space(other.name_space),
		name(other.name) {	}

	/**
	 * The constructor that auto-initializes the type information
	 */
	template <typename T>
	explicit type_descriptor():
		index(typeid(T)),
		type_id(type_id_name<T>()),
		name_space(type_namespace<T>()),
		name(type_short_name<T>()) { }


	/*!
	 * No assignment operator
	 */
	self_type &operator=(const self_type &other) = delete;

	/*!
	 * The type index from typeid()
	 */
	const std::type_index index;

	/*!
	 * The fully-qualified type identifier name, e.g. "tmx::support::types::type_descriptor"
	 */
	const std::string_view type_id;

	/*!
	 * The namespace that the type belongs to, e.g. "tmx::support::types"
	 */
	const std::string_view name_space;

	/*!
	 * The non-qualified (short-hand) name for the type, e.g. "int" or "type_descriptor"
	 */
	const std::string_view name;
};

} /* End namespace types */
} /* End namespace support */
} /* End namespace tmx */


#endif /* INCLUDE_TMX_COMMON_TYPES_TYPE_DESCRIPTOR_HPP_ */
