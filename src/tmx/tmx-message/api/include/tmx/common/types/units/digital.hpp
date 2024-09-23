/*!
 * Copyright (c) 2021 Battelle Memorial Institute
 *
 * All Rights Reserved.
 *
 * @file Digital.hpp
 *
 *  Created on: Jun 11, 2021
 *      @author: Gregory M. Baumgardner
 */

#ifndef INCLUDE_TMX_SUPPORT_UNITS_DIGITAL_HPP_
#define INCLUDE_TMX_SUPPORT_UNITS_DIGITAL_HPP_

#include <tmx/support/units.hpp>

namespace tmx {
namespace support {
namespace units {

// Digital Ratios.  Nominal unit is the byte
enum class Digital
{
		bits,
		kbit,
		Mbit,
		Gbit,
		Tbit,
		Pbit,
		bytes,
		kB,
		MB,
		GB,
		TB,
		PB
};

} /* End namespace units */

template <> struct unit_ratio_t<units::Digital, units::Digital::bits> { typedef std::ratio<8, 1> type; };
template <> struct unit_base_t<units::Digital, units::Digital::kbit> {
	static constexpr double value = factor<units::Digital, units::Digital::kB>::value;
};
template <> struct unit_ratio_t<units::Digital, units::Digital::kbit> { typedef std::ratio<8, 1> type; };
template <> struct unit_base_t<units::Digital, units::Digital::Mbit> {
	static constexpr double value = factor<units::Digital, units::Digital::MB>::value;
};
template <> struct unit_ratio_t<units::Digital, units::Digital::Mbit> { typedef std::ratio<8, 1> type; };
template <> struct unit_base_t<units::Digital, units::Digital::Gbit> {
	static constexpr double value = factor<units::Digital, units::Digital::GB>::value;
};
template <> struct unit_ratio_t<units::Digital, units::Digital::Gbit> { typedef std::ratio<8, 1> type; };
template <> struct unit_base_t<units::Digital, units::Digital::Tbit> {
	static constexpr double value = factor<units::Digital, units::Digital::TB>::value;
};
template <> struct unit_ratio_t<units::Digital, units::Digital::Tbit> { typedef std::ratio<8, 1> type; };
template <> struct unit_base_t<units::Digital, units::Digital::Pbit> {
	static constexpr double value = factor<units::Digital, units::Digital::PB>::value;
};
template <> struct unit_ratio_t<units::Digital, units::Digital::Pbit> { typedef std::ratio<8, 1> type; };
template <> struct unit_ratio_t<units::Digital, units::Digital::kB> { typedef std::kilo type; };
template <> struct unit_ratio_t<units::Digital, units::Digital::MB> { typedef std::mega type; };
template <> struct unit_ratio_t<units::Digital, units::Digital::GB> { typedef std::giga type; };
template <> struct unit_ratio_t<units::Digital, units::Digital::TB> { typedef std::tera type; };
template <> struct unit_ratio_t<units::Digital, units::Digital::PB> { typedef std::peta type; };

} /* End namespace support */
} /* End namespace tmx */



#endif /* INCLUDE_TMX_SUPPORT_UNITS_DIGITAL_HPP_ */
