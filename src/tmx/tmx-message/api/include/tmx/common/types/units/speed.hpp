/*!
 * Copyright (c) 2021 Battelle Memorial Institute
 *
 * All Rights Reserved.
 *
 * @file Speed.hpp
 *
 *  Created on: Jun 11, 2021
 *      @author: Gregory M. Baumgardner
 */

#ifndef INCLUDE_TMX_SUPPORT_UNITS_SPEED_HPP_
#define INCLUDE_TMX_SUPPORT_UNITS_SPEED_HPP_

#include <tmx/support/units/distance.hpp>
#include <tmx/support/units/time.hpp>
#include <tmx/support/units.hpp>

namespace tmx {
namespace support {
namespace units {

// Speed Ratios.  Nominal unit is mps (m/s)
enum class Speed
{
		mps,
		kph,
		ftpers,
		mph,
		knots
};

} /* End namespace units */

template <> struct unit_ratio_t<units::Speed, units::Speed::kph> {
	typedef typename std::ratio_divide<
			std::kilo, ratio<units::Time, units::Time::hrs>
	>::type type;
};
template <> struct unit_ratio_t<units::Speed, units::Speed::mph> {
	typedef typename std::ratio_divide<
			ratio<units::Distance, units::Distance::mi>, ratio<units::Time, units::Time::hrs>
	>::type type;
};
template <> struct unit_ratio_t<units::Speed, units::Speed::ftpers> {
	typedef ratio<units::Distance, units::Distance::ft> type;
};
template <> struct unit_base_t<units::Speed, units::Speed::knots> {
	static constexpr double value = factor<units::Speed, units::Speed::kph>::value;
};
template <> struct unit_ratio_t<units::Speed, units::Speed::knots> { typedef std::ratio<1852, 1000> type; };

} /* End namespace support */
} /* End namespace tmx */



#endif /* INCLUDE_TMX_SUPPORT_UNITS_SPEED_HPP_ */
