/*!
 * Copyright (c) 2021 Battelle Memorial Institute
 *
 * All Rights Reserved.
 *
 * @file Area.hpp
 *
 *  Created on: Jun 11, 2021
 *      @author: Gregory M. Baumgardner
 */

#ifndef INCLUDE_TMX_SUPPORT_UNITS_AREA_HPP_
#define INCLUDE_TMX_SUPPORT_UNITS_AREA_HPP_

#include <tmx/support/units/distance.hpp>
#include <tmx/support/units.hpp>

namespace tmx {
namespace support {
namespace units {

enum class Area
{
		sqcm,
		sqm,
		sqkm,
		ha,
		sqin,
		sqft,
		sqyd,
		sqmi,
		acre
};

} /* End namespace units */

template <> struct unit_ratio_t<units::Area, units::Area::sqcm> {
	typedef std::ratio_multiply<std::centi, std::centi>::type type;
};

template <> struct unit_ratio_t<units::Area, units::Area::sqkm> {
	typedef std::ratio_multiply<std::kilo, std::kilo>::type type;
};

template <> struct unit_ratio_t<units::Area, units::Area::ha> {
	typedef std::ratio<10000, 1> type;
};

template <> struct unit_ratio_t<units::Area, units::Area::sqin> {
	typedef typename std::ratio_multiply<
		ratio<units::Distance, units::Distance::in>, ratio<units::Distance, units::Distance::in>
	>::type type;
};

template <> struct unit_base_t<units::Area, units::Area::sqft> {
	constexpr double value = factor<units::Area, units::Area::sqin>::value;
};

template <> struct unit_ratio_t<units::Area, units::Area::sqft> {
	typedef typename std::ratio_multiply<
		ratio<units::Distance, units::Distance::ft>, ratio<units::Distance, units::Distance::ft>
	>::type type;
};

template <> struct unit_base_t<units::Area, units::Area::sqyd> {
	constexpr double value = factor<units::Area, units::Area::sqft>::value;
};

template <> struct unit_ratio_t<units::Area, units::Area::sqyd> {
	typedef typename std::ratio_multiply<
		ratio<units::Distance, units::Distance::yd>, ratio<units::Distance, units::Distance::yd>
	>::type type;
};

template <> struct unit_base_t<units::Area, units::Area::sqmi> {
	constexpr double value = factor<units::Area, units::Area::sqft>::value;
};

template <> struct unit_ratio_t<units::Area, units::Area::sqmi> {
	typedef typename std::ratio_multiply<
		ratio<units::Distance, units::Distance::mi>, ratio<units::Distance, units::Distance::mi>
	>::type type;
};

template <> struct unit_base_t<units::Area, units::Area::acre> {
	constexpr double value = factor<units::Area, units::Area::sqyd>::value;
};

template <> struct unit_ratio_t<units::Area, units::Area::acre> {
	typedef std::ratio<4840, 1> type;
};

} /* End namespace support */
} /* End namespace tmx */



#endif /* INCLUDE_TMX_SUPPORT_UNITS_AREA_HPP_ */
