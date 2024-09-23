/*!
 * Copyright (c) 2021 Battelle Memorial Institute
 *
 * All Rights Reserved.
 *
 * @file Mass.hpp
 *
 *  Created on: Jun 11, 2021
 *      @author: Gregory M. Baumgardner
 */

#ifndef INCLUDE_TMX_SUPPORT_UNITS_MASS_HPP_
#define INCLUDE_TMX_SUPPORT_UNITS_MASS_HPP_

#include <tmx/support/units.hpp>

namespace tmx {
namespace support {
namespace units {

// Mass Ratios.  Nominal unit is kilogram (kg)
enum class Mass
{
		mg,
		g,
		kg,
		tonne,
		oz,
		lb,
		ton,
		st
};

} /* End namespace units */

template <> struct unit_ratio_t<units::Mass, units::Mass::mg> { typedef std::micro type; };
template <> struct unit_ratio_t<units::Mass, units::Mass::g> { typedef std::milli type; };
template <> struct unit_ratio_t<units::Mass, units::Mass::tonne> { typedef std::kilo type; };
template <> struct unit_ratio_t<units::Mass, units::Mass::lb> {
	typedef std::ratio<45359237, 100000000> type;
};
template <> struct unit_base_t<units::Mass, units::Mass::oz> {
	static constexpr double value = factor<units::Mass, units::Mass::lb>::value;
};
template <> struct unit_ratio_t<units::Mass, units::Mass::oz> { typedef std::ratio<1, 16> type; };
template <> struct unit_base_t<units::Mass, units::Mass::ton> {
	static constexpr double value = factor<units::Mass, units::Mass::lb>::value;
};
template <> struct unit_ratio_t<units::Mass, units::Mass::ton> { typedef std::ratio<2000, 1> type; };
template <> struct unit_base_t<units::Mass, units::Mass::st> {
	static constexpr double value = factor<units::Mass, units::Mass::lb>::value;
};
template <> struct unit_ratio_t<units::Mass, units::Mass::st> { typedef std::ratio<14, 1> type; };


} /* End namespace support */
} /* End namespace tmx */


#endif /* INCLUDE_TMX_SUPPORT_UNITS_MASS_HPP_ */
