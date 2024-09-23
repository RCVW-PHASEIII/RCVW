/*!
 * Copyright (c) 2021 Battelle Memorial Institute
 *
 * All Rights Reserved.
 *
 * @file Power.hpp
 *
 *  Created on: Jun 11, 2021
 *      @author: Gregory M. Baumgardner
 */

#ifndef INCLUDE_TMX_SUPPORT_UNITS_POWER_HPP_
#define INCLUDE_TMX_SUPPORT_UNITS_POWER_HPP_

#include <tmx/support/units/energy.hpp>
#include <tmx/support/units/time.hpp>
#include <tmx/support/units.hpp>

namespace tmx {
namespace support {
namespace units {

// Power Ratios.  Nominal unit is the Watt (W)
enum class Power
{
		mW,
		W,
		kW,
		MegaW,
		GW,
		ergspers,
		lbfpers,
		hp,
		kcalperhr,
		dBm
};

} /* End namespace units */

template <> struct unit_ratio_t<units::Power, units::Power::mW> { typedef std::milli type; };
template <> struct unit_ratio_t<units::Power, units::Power::kW> { typedef std::kilo type; };
template <> struct unit_ratio_t<units::Power, units::Power::MegaW> { typedef std::mega type; };
template <> struct unit_ratio_t<units::Power, units::Power::GW> { typedef std::giga type; };
template <> struct unit_ratio_t<units::Power, units::Power::ergspers> {
	typedef ratio<units::Energy, units::Energy::erg> type;
};
template <> struct unit_ratio_t<units::Power, units::Power::lbfpers> {
	typedef std::ratio<1355817948, 1000000000> type;
};
template <> struct unit_base_t<units::Power, units::Power::hp> {
	static constexpr double value = factor<units::Power, units::Power::lbfpers>::value;
};
template <> struct unit_ratio_t<units::Power, units::Power::hp> { typedef std::ratio<550, 1> type; };
template <> struct unit_ratio_t<units::Power, units::Power::kcalperhr> {
	typedef typename std::ratio_divide<
			ratio<units::Energy, units::Energy::kcal>::type, ratio<units::Time, units::Time::hrs>
	>::type type;
};
template <> struct unit_ratio_t<units::Power, units::Power::dBm> { typedef std::ratio<10, 1> type; };

template <> inline double Convert<units::Power, units::Power::W, units::Power::dBm>(double in) {
	return ::log10(in) * factor<units::Power, units::Power::dBm>::value + 30;
}

template <> inline double Convert<units::Power, units::Power::dBm, units::Power::W>(double in) {
	return ::pow(10, (in - 30) * (1.0 / factor<units::Power, units::Power::dBm>::value));
}

} /* End namespace support */
} /* End namespace tmx */



#endif /* INCLUDE_TMX_SUPPORT_UNITS_POWER_HPP_ */
