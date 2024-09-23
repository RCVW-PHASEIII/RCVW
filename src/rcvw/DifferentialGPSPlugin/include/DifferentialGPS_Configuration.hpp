/*!
 * Copyright (c) 2024 Battelle Memorial Institute
 *
 * All Rights Reserved.
 *
 * @file DifferentialGPS_Configuration.hpp
 *
 *  Created on: May 30, 2024
 *      @author: Noah Limes
 */

#ifndef INCLUDE_DIFFERENTIAL_GPS_CONFIGURATION_HPP_
#define INCLUDE_DIFFERENTIAL_GPS_CONFIGURATION_HPP_

static constexpr auto DIFFERENTIAL_GPS_CONFIGURATION_JSON = R"(
[
        {
            "key":"rtcm-version",
            "default":"3.3",
            "description":"Only apply incoming RTCM correction messages of the given version.  Set to 0 to disable RTCM corrections."
        },
        {
            "key":"device",
            "default":"/dev/ttyACM0",
            "description":"The serial device for the GPS to write the corrections to"
        }
	]
)";

#endif /* INCLUDE_DIFFERENTIAL_GPS_CONFIGURATION_HPP_ */
