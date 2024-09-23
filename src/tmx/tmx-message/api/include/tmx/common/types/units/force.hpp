/*!
 * Copyright (c) 2021 Battelle Memorial Institute
 *
 * All Rights Reserved.
 *
 * @file Force.hpp
 *
 *  Created on: Jun 11, 2021
 *      @author: Gregory M. Baumgardner
 */

#ifndef INCLUDE_TMX_SUPPORT_UNITS_FORCE_HPP_
#define INCLUDE_TMX_SUPPORT_UNITS_FORCE_HPP_

#include <tmx/support/units/acceleration.hpp>
#include <tmx/support/units/mass.hpp>
#include <tmx/support/units.hpp>

namespace tmx {
namespace support {
namespace units {

// Force/Weight Ratios.  Nominal unit is Newton (N or kg*m/s^2)
enum class Force
{
		N,
		dyn,
		lbf,
		kp
};

} /* End namespace units */

template <> struct unit_ratio_t<units::Force, units::Force::dyn> { typedef std::ratio<1, 100000> type; };
template <> struct unit_ratio_t<units::Force, units::Force::lbf> {
	typedef typename std::ratio_multiply<
			ratio<units::Acceleration, units::Acceleration::g>, ratio<units::Mass, units::Mass::lb>
	>::type type;
};
template <> struct unit_ratio_t<units::Force, units::Force::kp> { typedef std::ratio<980665, 100000> type; };


} /* End namespace support */
} /* End namespace tmx */



#endif /* INCLUDE_TMX_SUPPORT_UNITS_FORCE_HPP_ */
