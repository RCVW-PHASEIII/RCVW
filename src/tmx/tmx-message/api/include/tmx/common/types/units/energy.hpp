/*!
 * Copyright (c) 2021 Battelle Memorial Institute
 *
 * All Rights Reserved.
 *
 * @file Energy.hpp
 *
 *  Created on: Jun 11, 2021
 *      @author: Gregory M. Baumgardner
 */

#ifndef INCLUDE_TMX_SUPPORT_UNITS_ENERGY_HPP_
#define INCLUDE_TMX_SUPPORT_UNITS_ENERGY_HPP_

#include <tmx/support/units/time.hpp>
#include <tmx/support/units.hpp>

namespace tmx {
namespace support {
namespace units {

// Energy Ratios.  Nominal unit is the Joule (J)
enum class Energy
{
		J,
		kJ,
		cal,
		kcal,
		BTU,
		kWh,
		ftlb,
		eV,
		erg
};

} /* End namespace units */

template <> struct unit_ratio_t<units::Energy, units::Energy::kJ> { typedef std::kilo type; };
template <> struct unit_ratio_t<units::Energy, units::Energy::cal> { typedef std::ratio<4184, 1000> type; };
template <> struct unit_base_t<units::Energy, units::Energy::kcal> {
	static constexpr double value = factor<units::Energy, units::Energy::cal>::value;
};
template <> struct unit_ratio_t<units::Energy, units::Energy::kcal> { typedef std::kilo type; };
template <> struct unit_ratio_t<units::Energy, units::Energy::BTU> { typedef std::ratio<10543503, 10000> type; };
template <> struct unit_ratio_t<units::Energy, units::Energy::kWh> {
	typedef typename std::ratio_multiply<
				std::kilo, ratio<units::Time, units::Time::hrs>
			>::type type;
};
template <> struct unit_ratio_t<units::Energy, units::Energy::ftlb> {
	typedef std::ratio<13558179483314004, 10000000000000000> type;
};
template <> struct unit_base_t<units::Energy, units::Energy::eV> { static constexpr double value = 1e-19; };
template <> struct unit_ratio_t<units::Energy, units::Energy::eV> {
	typedef std::ratio<16021766208, 10000000000> type;
};
template <> struct unit_ratio_t<units::Energy, units::Energy::erg> { typedef std::ratio<1, 10000000> type; };


} /* End namespace support */
} /* End namespace tmx */



#endif /* INCLUDE_TMX_SUPPORT_UNITS_ENERGY_HPP_ */
