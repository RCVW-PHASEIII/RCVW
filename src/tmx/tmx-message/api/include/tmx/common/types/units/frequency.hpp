/*!
 * Copyright (c) 2021 Battelle Memorial Institute
 *
 * All Rights Reserved.
 *
 * @file Frequency.hpp
 *
 *  Created on: Jun 11, 2021
 *      @author: Gregory M. Baumgardner
 */

#ifndef INCLUDE_TMX_SUPPORT_UNITS_FREQUENCY_HPP_
#define INCLUDE_TMX_SUPPORT_UNITS_FREQUENCY_HPP_

#include <tmx/support/units/angle.hpp>
#include <tmx/support/units/time.hpp>
#include <tmx/support/units.hpp>

namespace tmx {
namespace support {
namespace units {

// Frequency Ratios.  Nominal unit is Hertz (Hz or cycles/s)
enum class Frequency
{
		Hz,
		kHz,
		MHz,
		GHz,
		rpm,
		piradpers,
		radpers
};

} /* End namespace units */

template <> struct unit_ratio_t<units::Frequency, units::Frequency::kHz> { typedef std::kilo type; };
template <> struct unit_ratio_t<units::Frequency, units::Frequency::MHz> { typedef std::mega type; };
template <> struct unit_ratio_t<units::Frequency, units::Frequency::GHz> { typedef std::giga type; };
template <> struct unit_ratio_t<units::Frequency, units::Frequency::rpm> {
	typedef typename std::ratio_divide<
				std::ratio<1, 1>, ratio<units::Time, units::Time::min>
			>::type type;
};
template <> struct unit_ratio_t<units::Frequency, units::Frequency::piradpers> { typedef std::ratio<1, 2> type; };
template <> struct unit_base_t<units::Frequency, units::Frequency::radpers> {
	constexpr double value = factor<units::Frequency, units::Frequency::piradpers>::value / units::pi;
};

} /* End namespace support */
} /* End namespace tmx */



#endif /* INCLUDE_TMX_SUPPORT_UNITS_FREQUENCY_HPP_ */
