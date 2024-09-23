/*!
 * Copyright (c) 2021 Battelle Memorial Institute
 *
 * All Rights Reserved.
 *
 * @file TmxTypeDescriptor.hpp
 *
 *  Created on: Aug 6, 2021
 *      @author: Gregory M. Baumgardner
 */

#ifndef TYPES_INCLUDE_TMX_COMMON_TMXTYPEDESCRIPTOR_HPP_
#define TYPES_INCLUDE_TMX_COMMON_TMXTYPEDESCRIPTOR_HPP_

#include <tmx/platform.hpp>

#include <memory>
#include <string>
#include <typeinfo>

namespace tmx {
namespace common {

/*!
 * @brief A descriptor class for TMX types to help with mapping from names
 *
 * Each descriptor belongs to a namespace, which is held within the registry
 * instance at which the descriptor was created.
 */
class TmxTypeDescriptor {
public:
	/*!
	 * @brief No default constructor
	 */
	TmxTypeDescriptor() = delete;

	/*!
	 * @brief Construct a new descriptor
	 *
	 * @param[in] instance The type instance
	 * @param[in] id The type identifier
	 * @param[in] name The fully-qualified type name
	 */
	TmxTypeDescriptor(std::shared_ptr<const void> instance,
			const std::type_info &id, std::string name) noexcept;

	/*!
	 * @brief Construct a new descriptor from a copy
	 *
	 * @param[in] The type descriptor to use as a base
	 */
	TmxTypeDescriptor(TmxTypeDescriptor const &) noexcept;

	/*!
	 * @brief Use the default destructor
	 */
	virtual ~TmxTypeDescriptor() = default;

	/*!
	 * @brief Get the prototype instance for the type
	 *
	 * Note that this can generally only be used if the type is
	 * already well-known, perhaps as a virtual interface class.
	 *
	 * @return The prototype instance for the type
	 */
	std::shared_ptr<const void> const get_instance() const noexcept;


	/*!
	 * @brief Get the prototype instance for the type as a _Tp pointer
	 *
	 * Note that this can generally only be used if the type is
	 * already well-known, perhaps as a virtual interface class.
	 *
	 * @return The prototype instance for the type, or null if the _Tp is invalid
	 */
	template <typename _Tp>
	std::shared_ptr<const _Tp> const as_instance() const noexcept {
		return std::static_pointer_cast<const _Tp>(this->get_instance());
	}

	/*!
	 * @return Information on the actual type, i.e. not the container class
	 */
	std::type_info const &get_typeid() const noexcept;

    /*!
     * @return The fully qualified name of the type, as a filesystem path
     */
    common::filesystem::path const &get_path() const noexcept;

    /*!
     * @return The fully qualified name of the type, as a string
     */
    std::string get_type_name() const noexcept;

    /*!
     * @return The non-qualified name of the type, as a string
     */
    std::string get_type_short_name() const noexcept;

    /*!
     * @return The namespace of the type, as a string
     */
    std::string get_type_namespace() const noexcept;

	/*!
	 * @return True if the descriptors are the same. False otherwise
	 */
	bool operator ==(TmxTypeDescriptor const &) const noexcept;

	/*!
	 * @return True if this descriptor is the same as the given typeid. False otherwise
	 */
	bool operator ==(std::type_info const &) const noexcept;

	/*!
	 * @return True if the descriptors are not the same. False otherwise
	 */
	bool operator !=(TmxTypeDescriptor const &) const noexcept;

	/*!
	 * @return True if this descriptor is not the same as the given typeid. False otherwise
	 */
	bool operator !=(std::type_info const &) const noexcept;

	/*!
	 * @return True if this descriptor is valid, i.e. not empty. Falsoe otherwise
	 */
	operator bool () const noexcept;

protected:
	std::shared_ptr<const void> _instance;
	const std::type_info &_id;
	const filesystem::path _path;
};

} /* End namespace common */
} /* End namespace tmx */

#endif /* TYPES_INCLUDE_TMX_COMMON_TMXTYPEDESCRIPTOR_HPP_ */
