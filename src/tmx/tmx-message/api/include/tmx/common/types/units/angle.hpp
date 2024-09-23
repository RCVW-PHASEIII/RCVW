/*!
 * Copyright (c) 2021 Battelle Memorial Institute
 *
 * All Rights Reserved.
 *
 * @file Angle.hpp
 *
 *  Created on: Jun 11, 2021
 *      @author: Gregory M. Baumgardner
 */

#ifndef INCLUDE_TMX_SUPPORT_UNITS_ANGLE_HPP_
#define INCLUDE_TMX_SUPPORT_UNITS_ANGLE_HPP_

#include <tmx/support/units.hpp>


namespace tmx {
namespace support {
namespace units {

constexpr long double pi = 3.1415926535897932384626433832795028841971693993751058209749445923078164062862089986280348253421170679;

// Angle Ratios.  Nominal unit is degree (\u2070)
enum class Angle
{
		deg,
		pirad,
		rad,
		grad
};

} /* End namespace units */

template <> struct unit_ratio_t<units::Angle, units::Angle::pirad> {
	typedef typename std::ratio<180, 1>::type type;
};

template <> struct unit_base_t<units::Angle, units::Angle::rad> {
	constexpr double value = factor<units::Angle, units::Angle::pirad>::value / units::pi;
};

template <> struct unit_ratio_t<units::Angle, units::Angle::grad> {
	typedef typename std::ratio<9, 10>::type type;
};

} /* End namespace support */
} /* End namespace tmx */



#endif /* INCLUDE_TMX_SUPPORT_UNITS_ANGLE_HPP_ */
