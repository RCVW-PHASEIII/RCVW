/*!
 * Copyright (c) 2021 Battelle Memorial Institute
 *
 * All Rights Reserved.
 *
 * @file Acceleration.hpp
 *
 *  Created on: Jun 11, 2021
 *      @author: Gregory M. Baumgardner
 */

#ifndef INCLUDE_TMX_COMMON_TYPES_UNITS_ACCELERATION_HPP_
#define INCLUDE_TMX_COMMON_TYPES_UNITS_ACCELERATION_HPP_

#include <tmx/common/types/units/distance.hpp>
#include <tmx/common/types/units/time.hpp>
#include <tmx/common/types/units/units.hpp>

namespace tmx {
namespace common {
namespace types {
namespace units {

// Acceleration Ratios.  Nominal unit is meters per second per second (m/s/s or m/s^2)
enum class Acceleration
{
		Gal,
		mperspers,
		kmperhrperhr,
		ftperspers,
		miperhrperhr,
		g
};

template <> struct unit_ratio_t<units::Acceleration, units::Acceleration::Gal> {
	typedef std::centi type;
};

template <> struct unit_ratio_t<units::Acceleration, units::Acceleration::kmperhrperhr> {
	typedef typename std::ratio_divide<std::kilo,
			typename std::ratio_multiply<
				ratio<units::Time, units::Time::hrs>, ratio<units::Time, units::Time::hrs>
			>::type
	>::type type;
};

template <> struct unit_ratio_t<units::Acceleration, units::Acceleration::ftperspers> {
	typedef typename ratio<units::Distance, units::Distance::ft>::type type;
};

template <> struct unit_ratio_t<units::Acceleration, units::Acceleration::miperhrperhr> {
	typedef typename std::ratio_divide<ratio<units::Distance, units::Distance::mi>,
			typename std::ratio_multiply<
				ratio<units::Time, units::Time::hrs>, ratio<units::Time, units::Time::hrs>
			>::type
	>::type type;
};

template <> struct unit_ratio_t<units::Acceleration, units::Acceleration::g> {
	typedef typename std::ratio<980665, 100000>::type type;
};

} /* End namespace units */
} /* End namespace types */
} /* End namespace common */
} /* End namespace tmx */


#endif /* INCLUDE_TMX_COMMON_TYPES_UNITS_ACCELERATION_HPP_ */
