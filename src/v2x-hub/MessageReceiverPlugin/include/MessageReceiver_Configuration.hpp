/*!
 * Copyright (c) 2024 Battelle Memorial Institute
 *
 * All Rights Reserved.
 *
 * @file MessageReceiver_Configuration.hpp
 *
 *  Created on: May 16, 2024
 *      @author: Noah Limes
 */

#ifndef INCLUDE_MESSAGE_RECEIVER_CONFIGURATION_HPP_
#define INCLUDE_MESSAGE_RECEIVER_CONFIGURATION_HPP_

static constexpr auto MESSAGE_RECEIVER_CONFIGURATION_JSON = R"(
[
    {
        "key": "status-Hz",
        "default": 1,
        "description": "Publication rate (Hz) of MessageReceiver status messages."
    },
    {
        "key": "decode-and-forward",
        "default": false,
        "description": "Set the flag to decode and forward a received J2735 message to the external forwarding channel, if one exists."
    },
    {
        "key": "enable-j2735",
        "default": true,
        "description": "Accept and route incoming J2735 messages from an external source."
    },
    {
        "key": "enable-sim-bsm",
        "default": true,
        "description":"Accept and route incoming BSM messages from the V2X Hub simulator."
    },
    {
        "key":"enable-sim-srm",
        "default": true,
        "description":"Accept and route incoming SRM messages from the V2X Hub simulator."
    },
    {
        "key":"enable-sim-tpv",
        "default": true,
        "description":"Route incoming GPS time-position-velocity (TPV) messages from the V2X Hub simulator."
    }
]
)";

#endif /* INCLUDE_MESSAGE_RECEIVER_CONFIGURATION_HPP_ */
