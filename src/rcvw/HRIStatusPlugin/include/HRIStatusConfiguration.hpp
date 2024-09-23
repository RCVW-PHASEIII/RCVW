/*!
 * Copyright (c) 2023 Battelle Memorial Institute
 *
 * All Rights Reserved.
 *
 * @file HRIStatusConfiguration.hpp
 *
 *  Created on: Nov 17, 2023
 *      @author: Gregory M. Baumgardner
 */

#ifndef INCLUDE_HRISTATUSCONFIGURATION_HPP_
#define INCLUDE_HRISTATUSCONFIGURATION_HPP_

static constexpr auto HRI_STATUS_CONFIG_JSON = R"(
[
		{
			"key":"Frequency",
			"default":"100",
			"description":"The frequency to send the SPAT message in milliseconds."
		},
		{
			"key":"RailPinNumber",
			"default":"0",
			"description":"The digital I/O pin that the rail signal is on. Only used if PortName is missing."
		},
		{
		    "key":"PortName",
		    "default":"/dev/ttyS0",
		    "description":"The serial port to use for communication, blank if not using serial port."
		},
		{
		    "key":"LaneMap",
		    "description":"Maps the signal groups in the SPaT message to either tracked or vehicle lanes."
		},
		{
		    "key":"IntersectionID",
		    "description":"The intersection id to use in the spat message."
		},
		{
		    "key":"IntersectionName",
		    "description":"The intersection name to use in the spat message."
		},
		{
		    "key":"AlwaysSend",
		    "default":true,
		    "description":"Always send the SPaT message. Otherwise only send when BSMs or Preempt is true."
		},
		{
		    "key":"SerialDataTimeout",
		    "default":"1500",
		    "description":"The timeout to mark serial data as invalid in milliseconds."
		}
	]
)";

#endif /* INCLUDE_HRISTATUSCONFIGURATION_HPP_ */