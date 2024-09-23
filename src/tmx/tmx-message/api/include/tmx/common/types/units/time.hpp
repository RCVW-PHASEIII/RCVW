/*!
 * Copyright (c) 2021 Battelle Memorial Institute
 *
 * All Rights Reserved.
 *
 * @file Time.hpp
 *
 *  Created on: Jun 11, 2021
 *      @author: Gregory M. Baumgardner
 */

#ifndef INCLUDE_TMX_COMMON_TYPES_UNITS_TIME_HPP_
#define INCLUDE_TMX_COMMON_TYPES_UNITS_TIME_HPP_

namespace tmx {
namespace common {
namespace types {
namespace units {

enum class Time
{
		ns,
		us,
		ms,
		s,
		min,
		hrs,
		days,
		weeks,
		months,
		years
};

} /* End namespace utils */
} /* End namespace types */
} /* End namespace common */
} /* End namespace tmx */


#endif /* INCLUDE_TMX_COMMON_TYPES_UNITS_TIME_HPP_ */
