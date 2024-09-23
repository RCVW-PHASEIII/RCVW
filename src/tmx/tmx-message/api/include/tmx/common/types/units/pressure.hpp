/*!
 * Copyright (c) 2021 Battelle Memorial Institute
 *
 * All Rights Reserved.
 *
 * @file Pressure.hpp
 *
 *  Created on: Jun 11, 2021
 *      @author: Gregory M. Baumgardner
 */

#ifndef INCLUDE_TMX_SUPPORT_UNITS_PRESSURE_HPP_
#define INCLUDE_TMX_SUPPORT_UNITS_PRESSURE_HPP_

#include <tmx/support/units.hpp>

namespace tmx {
namespace support {
namespace units {

// Pressure Ratios.  Nominal unit is the Pascal (Pa or N/m^2)
enum class Pressure
{
		Pa,
		bar,
		atm,
		psi,
		mmhg
};

} /* End namespace units */

template <> struct unit_ratio_t<units::Pressure, units::Pressure::bar> { typedef std::ratio<100000, 1> type; };
template <> struct unit_ratio_t<units::Pressure, units::Pressure::atm> { typedef std::ratio<101325, 1> type; };
template <> struct unit_ratio_t<units::Pressure, units::Pressure::psi> {
	typedef std::ratio<44482216152605, 6451600000> type;
};
template <> struct unit_base_t<units::Pressure, units::Pressure::mmhg> {
	static constexpr double value = factor<units::Pressure, units::Pressure::atm>::value;
};
template <> struct unit_ratio_t<units::Pressure, units::Pressure::mmhg> { typedef std::ratio<1, 760> type; };

} /* End namespace support */
} /* End namespace tmx */



#endif /* INCLUDE_TMX_SUPPORT_UNITS_PRESSURE_HPP_ */
