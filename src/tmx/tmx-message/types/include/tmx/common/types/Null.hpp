/*!
 * Copyright (c) 2021 Battelle Memorial Institute
 *
 * All Rights Reserved.
 *
 * @file Null.hpp
 *
 * Contains the C++ implementation for the TMX
 * Null type.
 *
 *  Created on: Jun 19, 2021
 *      @author: Gregory M. Baumgardner
 */

#ifndef TYPES_INCLUDE_TMX_COMMON_TYPES_NULL_HPP_
#define TYPES_INCLUDE_TMX_COMMON_TYPES_NULL_HPP_

#include <tmx/platform.hpp>
#include <tmx/common/types/TmxDataType.hpp>

#include <cstddef>
#include <string>
#include <type_traits>

namespace tmx {
namespace common {
namespace types {

/*!
 * @brief A class for holding a null type
 */
class Null: public TmxDataType<std::nullptr_t> {
	typedef TmxDataType<std::nullptr_t> super;

public:
	/*!
	 * @brief The value type
	 */
	typedef typename super::value_type value_type;

	/*!
	 * @brief A self-reflective type
	 */
	typedef Null self_type;

	/*!
	 * @brief Constuct a Null type with a null pointer
	 *
	 */
	Null(value_type = nullptr): super(nullptr) { }

	/*!
	 * @return Just returns this object, which is already null
	 */
	self_type &operator=(const value_type &) noexcept {
		return *this;
	}

	/*!
	 * @return Just returns this object, which is already null
	 */
	self_type &operator=(value_type &&) noexcept {
		return *this;
	}

	/*!
	 * @return Just returns this object, which is already null
	 */
	self_type &operator=(const Null &) noexcept {
		return *this;
	}

	/*!
	 * @return Just returns this object, which is already null
	 */
	template <class _C>
	self_type &operator=(const _C *) noexcept {
		return *this;
	}

	/*!
	 * @return False for a null
	 */
	constexpr operator bool() const noexcept { return false; }

	/*!
	 * @return True since the comparison is to a null
	 */
	constexpr bool operator==(value_type const &) const noexcept {
		return true;
	}

	/*!
	 * @return True when comparing to another Null type
	 */
	constexpr bool operator==(const Null &) const noexcept {
		return true;
	}

	/*!
	 * @return False since the comparison is to a null
	 */
	constexpr bool operator!=(value_type const &) const noexcept {
		return false;
	}

	/*!
	 * @return False when comparing to another Null type
	 */
	constexpr bool operator!=(const Null &) const noexcept {
		return false;
	}

	/*!
	 * @return True if the pointer is null
	 */
	template <class _C>
	constexpr bool operator==(const _C *ptr) const noexcept {
		return !ptr;
	}

	/*!
	 * @return True if the pointer is not null
	 */
	template <class _C>
	constexpr bool operator!=(const _C *ptr) const noexcept {
		return ptr;
	}

	/*!
	 * @brief No bits are needed to store a null value
	 */
	static constexpr std::uint8_t numBytes = 0;

	/*!
	 * @brief No bits are needed to store a null value
	 */
	static constexpr std::uint8_t numBits = 0;

	/*!
	 * @return The number of bytes needed to store this TMX type
	 */
	std::size_t get_byte_count() const noexcept override {
		return numBytes;
	}

	/*!
	 * @return The number of bits needed to store this TMX type
	 */
	std::size_t get_bit_count() const noexcept override {
		return numBits;
	}
};

/*!
 * @brief null_type is an alias to the TMX Null type
 */
typedef Null null_type;

template <>
struct TmxType<Null, void> {
	typedef std::true_type TMX;
	typedef Null type;
    static constexpr auto name = type_short_name<type>();
};

template <>
struct TmxType<typename Null::value_type, void> {
	typedef std::true_type TMX;
	typedef Null type;
    static constexpr auto name = type_short_name<type>();
};

using TmxNullTypes = TmxTypes<Null>;

template <>
inline Null make_type<Null>(Null const &) {
    return { };
}

} /* End namespace types */
} /* End namespace common */
} /* End namespace tmx */

#endif /* TYPES_INCLUDE_TMX_COMMON_TYPES_NULL_HPP_ */
