/*!
 * Copyright (c) 2021 Battelle Memorial Institute
 *
 * All Rights Reserved.
 *
 * @file Distance.hpp
 *
 *  Created on: Jun 11, 2021
 *      @author: Gregory M. Baumgardner
 */

#ifndef INCLUDE_TMX_COMMON_TYPES_UNITS_DISTANCE_HPP_
#define INCLUDE_TMX_COMMON_TYPES_UNITS_DISTANCE_HPP_

#include <tmx/common/types/units/units.hpp>

namespace tmx {
namespace common {
namespace types {
namespace units {

// Distance Ratios.  Nominal unit is meter (m)
enum class Distance
{
		nm,
		um,
		mm,
		cm,
		dm,
		m,
		km,
		in,
		ft,
		yd,
		mi
};

template <> struct _unit_ratio_t<units::Distance, units::Distance::nm> { typedef std::nano type; };
template <> struct _unit_ratio_t<units::Distance, units::Distance::um> { typedef std::micro type; };
template <> struct _unit_ratio_t<units::Distance, units::Distance::mm> { typedef std::milli type; };
template <> struct _unit_ratio_t<units::Distance, units::Distance::cm> { typedef std::centi type; };
template <> struct _unit_ratio_t<units::Distance, units::Distance::dm> { typedef std::deci type; };
template <> struct _unit_ratio_t<units::Distance, units::Distance::km> { typedef std::kilo type; };

template <> struct _unit_ratio_t<units::Distance, units::Distance::in> {
	typedef support::rational<254, 10000> type;
};

template <> struct unit_base_t<units::Distance, units::Distance::ft> {
	constexpr double value = factor<units::Distance, units::Distance::in>::value;
};

template <> struct _unit_ratio_t<units::Distance, units::Distance::ft> {
	typedef ratio<units::Distance, support::rational<12> type;
};

template <> struct unit_base_t<units::Distance, units::Distance::yd> {
	constexpr double value = factor<units::Distance, units::Distance::ft>::value;
};

template <> struct _unit_ratio_t<units::Distance, units::Distance::yd> {
	typedef std::ratio<3, 1> type;
};

template <> struct unit_base_t<units::Distance, units::Distance::mi> {
	constexpr double value = factor<units::Distance, units::Distance::ft>::value;
};

template <> struct _unit_ratio_t<units::Distance, units::Distance::mi> {
	typedef std::ratio<5280, 1> type;
};


} /* End namespace units */
} /* End namespace types */
} /* End namespace common */
} /* End namespace tmx */


#endif /* INCLUDE_TMX_COMMON_TYPES_UNITS_DISTANCE_HPP_ */
