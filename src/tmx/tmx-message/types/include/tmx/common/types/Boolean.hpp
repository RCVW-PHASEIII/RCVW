/*!
 * Copyright (c) 2021 Battelle Memorial Institute
 *
 * All Rights Reserved.
 *
 * @file Boolean.hpp
 *
 * Contains the C++ implementation for the TMX
 * Null type.
 *
 *  Created on: Jun 19, 2021
 *      @author: Gregory M. Baumgardner
 */

#ifndef TYPES_INCLUDE_TMX_COMMON_TYPES_BOOLEAN_HPP_
#define TYPES_INCLUDE_TMX_COMMON_TYPES_BOOLEAN_HPP_

#include <tmx/platform.hpp>
#include <tmx/common/types/TmxDataType.hpp>

#include <string>
#include <utility>
#include <type_traits>

namespace tmx {
namespace common {
namespace types {

/*!
 * @brief A class for holding a boolean type
 */
class Boolean: public TmxDataType<bool> {
	typedef TmxDataType<bool> super;

public:
	/*!
	 * @brief The value type
	 */
	typedef typename super::value_type value_type;

	/*!
	 * @brief A self-reflective type
	 */
	typedef Boolean self_type;

	Boolean() noexcept: super() { }
	Boolean(value_type &&t) noexcept: super(std::forward<value_type>(t)) { }
	Boolean(value_type &t) noexcept: super(t) { };
	Boolean(value_type const &t) noexcept: Boolean(bool(t)) { }

	using super::operator =;

	/*!
	 * @brief A copy of the boolean contained in this type
	 *
	 * This override is necessary because the "true" value
	 * could mean anything not zero. This function forces a
	 * unified result so that any casting to integers would
	 * be consistently 1 or 0.
	 *
	 * @return True if this value contains true, false otherwise
	 */
	operator bool () const noexcept override {
		return (bool)(super::operator bool() ? 1 : 0);
	}

	/*!
	 * @return A true value
	 */
	static self_type true_value() { return { true }; }

	/*!
	 * @return A false value
	 */
	static self_type false_value() { return { false }; }

	/*!
	 * @return The complement of this value
	 */
	value_type operator!() const noexcept {
		return !((value_type)(*this));
	}

	/*!
	 * @brief Only one byte is needed to store a boolean value
	 */
	static constexpr std::uint8_t numBytes = 1;

	/*!
	 * @brief Only one bit is needed to store a boolean value
	 */
	static constexpr std::uint8_t numBits = 1;

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
 * @brief boolean_type is an alias to the TMX Boolean type
 */
typedef Boolean boolean_type;

template <>
struct TmxType<Boolean, void> {
	typedef std::true_type TMX;
	typedef Boolean type;
    static constexpr auto name = type_short_name<type>();
};

template <>
struct TmxType<typename Boolean::value_type, void> {
	typedef std::true_type TMX;
	typedef Boolean type;
    static constexpr auto name = type_short_name<type>();
};

using TmxBooleanTypes = TmxTypes<Boolean>;

} /* End namespace types */
} /* End namespace common */
} /* End namespace tmx */


#endif /* TYPES_INCLUDE_TMX_COMMON_TYPES_BOOLEAN_HPP_ */
