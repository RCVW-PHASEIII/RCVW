/*!
 * Copyright (c) 2021 Battelle Memorial Institute
 *
 * All Rights Reserved.
 *
 * @file Economy.hpp
 *
 *  Created on: Jun 11, 2021
 *      @author: Gregory M. Baumgardner
 */

#ifndef INCLUDE_TMX_SUPPORT_UNITS_ECONOMY_HPP_
#define INCLUDE_TMX_SUPPORT_UNITS_ECONOMY_HPP_

#include <tmx/support/units/distance.hpp>
#include <tmx/support/units/volume.hpp>
#include <tmx/support/units.hpp>

namespace tmx {
namespace support {
namespace units {

// Economy Ratios.  Nominal unit is kilometers per liter (km/l)
enum class Economy
{
		kmperl,
		mpg
};

} /* End namespace units */

template struct unit_ratio_t<units::Economy, units::Economy::mpg> {
	typedef typename std::ratio_divide< ratio<units::Distance, units::Distance::mi>,
			typename std::ratio_multiply<
				std::kilo, ratio<units::Volume, units::Volume::gal>
			>::type
		>::type type;
};


} /* End namespace support */
} /* End namespace tmx */



#endif /* INCLUDE_TMX_SUPPORT_UNITS_ECONOMY_HPP_ */
