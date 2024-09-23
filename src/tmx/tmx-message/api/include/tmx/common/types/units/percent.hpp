/*!
 * Copyright (c) 2021 Battelle Memorial Institute
 *
 * All Rights Reserved.
 *
 * @file Percent.hpp
 *
 *  Created on: Jun 11, 2021
 *      @author: Gregory M. Baumgardner
 */

#ifndef INCLUDE_TMX_SUPPORT_UNITS_PERCENT_HPP_
#define INCLUDE_TMX_SUPPORT_UNITS_PERCENT_HPP_

#include <tmx/support/units.hpp>

namespace tmx {
namespace support {
namespace units {

// Percentage Ratios.  Nominal unit is the percent (%)
enum class Percent
{
		decimal,
		pct
};

} /* End namespace units */

template <> struct unit_ratio_t<units::Percent, units::Percent::decimal> { typedef std::centi type; };

} /* End namespace support */
} /* End namespace tmx */



#endif /* INCLUDE_TMX_SUPPORT_UNITS_PERCENT_HPP_ */
