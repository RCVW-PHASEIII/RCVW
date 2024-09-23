/*!
 * Copyright (c) 2021 Battelle Memorial Institute
 *
 * All Rights Reserved.
 *
 * @file Temperature.hpp
 *
 *  Created on: Jun 11, 2021
 *      @author: Gregory M. Baumgardner
 */

#ifndef INCLUDE_TMX_COMMON_TYPES_UNITS_TEMPERATURE_HPP_
#define INCLUDE_TMX_COMMON_TYPES_UNITS_TEMPERATURE_HPP_

#include <tmx/common/types/units/units.hpp>

namespace tmx {
namespace common {
namespace types {
namespace units {

// Temperature Ratios.  Nominal unit is degrees Celsius (⁰C)
enum class Temperature
{
		C,
		F,
		K
};

template <> struct _unit_conversion_fn_t<units::Temperature, units::Temperature::F> {
	typedef support::linear_fn< std::ratio<9, 5>, support::rational<32> > type;
};

template <> struct _unit_conversion_fn_t<units::Temperature, Temperature::K> {
	typedef support::linear_fn< support::ratio_identity, support::rational<-273.15> > type;
};


} /* End namespace units */
} /* End namespace types */
} /* End namespace common */
} /* End namespace tmx */


#endif /* INCLUDE_TMX_COMMON_TYPES_UNITS_TEMPERATURE_HPP_ */
