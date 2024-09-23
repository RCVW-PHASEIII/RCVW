/*!
 * Copyright (c) 2024 Battelle Memorial Institute
 *
 * All Rights Reserved.
 *
 * @file GNSS_Configuration.hpp
 *
 *  Created on: May 5, 2024
 *      @author: Noah Limes
 */

#ifndef INCLUDE_GNSS_CONFIGURATION_HPP_
#define INCLUDE_GNSS_CONFIGURATION_HPP_

static constexpr auto GNSS_CONFIGURATION_JSON = R"(
[
        {
        	"key":"SEND-NMEA",
        	"default":"true",
        	"description":"Should route raw NMEA traffic through TMX"
        },
        {
        	"key":"LATCH-SPEED",
        	"default":"0.1",
        	"description":"Speed at which vehicle is considered stopped"
        },
        {
        	"key":"LOCATION-HZ",
        	"default":"10",
        	"description":"Publication rate (Hz) of TMX Location messages"
        },
        {
        	"key":"STATUS-HZ",
        	"default":"10",
        	"description":"Publication rate (Hz) of GNSSPlugin status messages"
        },
        {
        	"key":"CFG-RATE-MEAS",
        	"default":"100",
        	"description":"Nominal time between GNSS measurements for a u-Blox device, in milliseconds."
        },
        {
        	"key":"CFG-NAVSPG-DYNMODEL",
        	"default":"4",
        	"description":"Dynamic platform model for a u-Blox device. Common values are 0=Portable, 2=Stationary, 3=Pedestrian, 4=Automotive, 5=Sea"
        },
        {
        	"key":"CFG-NMEA-PROTVER",
        	"default":"41",
        	"description":"NMEA protocl version to use for a u-Blox device. Common values are 21=2.1, 23=2.3, 40=4.0, 41=4.1, 42=4.2"
        },
        {
        	"key":"CFG-NMEA-HIGHPREC",
        	"default":"1",
        	"description":"Enable high precision mode for a u-Blox device."
        },
        {
        	"key":"CFG-RATE-TIMEREF",
        	"default":"1",
        	"description":"Time system to which measurements are aligned for a u-Blox device. Common values are 0=UTC time, 1=GPS time, 2=GLONASS time"
        },
        {
        	"key":"CFG-UART1-BAUDRATE",
        	"default":"460800",
        	"description":"The baud rate that should be configured on the UART1 interface for a u-Blox device"
        },
        {
        	"key":"CFG-UART1INPROT-UBX",
        	"default":"1",
        	"description":"Flag to indicate if UBX should be an input protocol on the UART1 interface for a u-Blox device"
        },
        {
        	"key":"CFG-UART1INPROT-NMEA",
        	"default":"0",
        	"description":"Flag to indicate if NMEA should be an input protocol on the UART1 interface for a u-Blox device"
        },
        {
        	"key":"CFG-UART1INPROT-RTCM3X",
        	"default":"1",
        	"description":"Flag to indicate if RTCM3X should be an input protocol on the UART1 interface for a u-Blox device"
        },
        {
        	"key":"CFG-UART1OUTPROT-UBX",
        	"default":"1",
        	"description":"Flag to indicate if UBX should be an output protocol on the UART1 interface for a u-Blox device"
        },
        {
        	"key":"CFG-UART1OUTPROT-NMEA",
        	"default":"1",
        	"description":"Flag to indicate if NMEA should be an output protocol on the UART1 interface for a u-Blox device"
        },
        {
        	"key":"CFG-UART1OUTPROT-RTCM3X",
        	"default":"0",
        	"description":"Flag to indicate if RTCM3X should be an output protocol on the UART1 interface for a u-Blox device"
        },
        {
        	"key":"CFG-USBINPROT-UBX",
        	"default":"1",
        	"description":"Flag to indicate if UBX should be an input protocol on the USB interface for a u-Blox device"
        },
        {
        	"key":"CFG-USBINPROT-NMEA",
        	"default":"1",
        	"description":"Flag to indicate if NMEA should be an input protocol on the USB interface for a u-Blox devicea"
        },
        {
        	"key":"CFG-USBINPROT-RTCM3X",
        	"default":"1",
        	"description":"Flag to indicate if RTCM3X should be an input protocol on the USB interface for a u-Blox device"
        },
        {
        	"key":"CFG-USBOUTPROT-UBX",
        	"default":"1",
        	"description":"Flag to indicate if UBX should be an output protocol on the USB interface for a u-Blox device"
        },
        {
        	"key":"CFG-USBOUTPROT-NMEA",
        	"default":"1",
        	"description":"Flag to indicate if NMEA should be an output protocol on the USB interface for a u-Blox device"
        },
        {
        	"key":"CFG-USBOUTPROT-RTCM3X",
        	"default":"0",
        	"description":"Flag to indicate if RTCM3X should be an output protocol on the USB interface for a u-Blox device"
        },
        {
        	"key":"CFG-MSGOUT-UBX_NAV_PVT_USB",
        	"default":"1",
        	"description":"Output rate of the navigation position-time-velocity message on the USB interface for a u-Blox device, per second, per epoch"
        },
        {
        	"key":"CFG-MSGOUT-UBX_NAV_SVIN_USB",
        	"default":"0",
        	"description":"Output rate of the navigation survey-in data message on the USB interface for a u-Blox device, per second, per epoch"
        },
        {
        	"key":"CFG-MSGOUT-RTCM_3X_TYPE1005_UART1",
        	"default":"0",
        	"description":"Output rate of the RTCM3 type 1005 message on the UART1 interface for a u-Blox device, per second, per epoch"
        },
        {
        	"key":"CFG-MSGOUT-RTCM_3X_TYPE1074_UART1",
        	"default":"0",
        	"description":"Output rate of the RTCM3 type 1074 message on the UART1 interface for a u-Blox device, per second, per epoch"
        },
        {
        	"key":"CFG-MSGOUT-RTCM_3X_TYPE1084_UART1",
        	"default":"0",
        	"description":"Output rate of the RTCM3 type 1084 message on the UART1 interface for a u-Blox device, per second, per epoch"
        },
        {
        	"key":"CFG-MSGOUT-RTCM_3X_TYPE1094_UART1",
        	"default":"0",
        	"description":"Output rate of the RTCM3 type 1094 message on the UART1 interface for a u-Blox device, per second, per epoch"
        },
        {
        	"key":"CFG-MSGOUT-RTCM_3X_TYPE1124_UART1",
        	"default":"0",
        	"description":"Output rate of the RTCM3 type 1124 message on the UART1 interface for a u-Blox device, per second, per epoch"
        },
        {
        	"key":"CFG-MSGOUT-RTCM_3X_TYPE1230_UART1",
        	"default":"0",
        	"description":"Output rate of the RTCM3 type 1230 message on the UART1 interface for a u-Blox device, per second, per epoch"
        },
        {
        	"key":"CFG-MSGOUT-RTCM_3X_TYPE1005_USB",
        	"default":"0",
        	"description":"Output rate of the RTCM3 type 1005 message on the USB interface for a u-Blox device, per second, per epoch"
        },
        {
        	"key":"CFG-MSGOUT-RTCM_3X_TYPE1074_USB",
        	"default":"0",
        	"description":"Output rate of the RTCM3 type 1074 message on the USB interface for a u-Blox device, per second, per epoch"
        },
        {
        	"key":"CFG-MSGOUT-RTCM_3X_TYPE1084_USB",
        	"default":"0",
        	"description":"Output rate of the RTCM3 type 1084 message on the USB interface for a u-Blox device, per second, per epoch"
        },
        {
        	"key":"CFG-MSGOUT-RTCM_3X_TYPE1094_USB",
        	"default":"0",
        	"description":"Output rate of the RTCM3 type 1094 message on the USB interface for a u-Blox device, per second, per epoch"
        },
        {
        	"key":"CFG-MSGOUT-RTCM_3X_TYPE1124_USB",
        	"default":"0",
        	"description":"Output rate of the RTCM3 type 1124 message on the USB interface for a u-Blox device, per second, per epoch"
        },
        {
        	"key":"CFG-MSGOUT-RTCM_3X_TYPE1230_USB",
        	"default":"0",
        	"description":"Output rate of the RTCM3 type 1230 message on the USB interface for a u-Blox device, per second, per epoch"
        },
        {
        	"key":"CFG-TMODE-MODE",
        	"default":"0",
        	"description":"Time mode receiver mode for a u-Blox device. Common values are 0=Disabled, 1=Survey in, 2=Fixed mode"
        },
        {
        	"key":"CFG-TMODE-POS_TYPE",
        	"default":"1",
        	"description":"Determines whether the antenna reference point (ARP) position is given in ECEF or LAT/LON/HEIGHT for a u-Blox device. Common values are 0=ECEF, 1=LLH"
        },
        {
        	"key":"CFG-TMODE-LAT",
        	"default":"0",
        	"description":"Latitude of the ARP position for a u-Blox device, in 1e-7 degrees"
        },
        {
        	"key":"CFG-TMODE-LON",
        	"default":"0",
        	"description":"Longitude of the ARP position for a u-Blox device, in 1e-7 degrees"
        },
        {
        	"key":"CFG-TMODE-HEIGHT",
        	"default":"0",
        	"description":"Height of the ARP position for a u-Blox device, in cm"
        },
        {
            "key":"NMEA-FILTERS",
            "default": [
                {
                    "key":"GGA",
                    "default":"true",
                    "description":""
                },
                {
                    "key":"PPA",
                    "default":"true",
                    "description":""
                },
                {
                    "key":"GGV",
                    "default":"true",
                    "description":""
                },
                {
                    "key":"RMC",
                    "default":"true",
                    "description":""
                },
                {
                    "key":"VTG",
                    "default":"true",
                    "description":""
                },
                {
                    "key":"GLL",
                    "default":true,
                    "description":""
                },
                {
                    "key":"GSA",
                    "default":"true",
                    "description":""
                }
            ]
        }
	]
)";

#endif /* INCLUDE_GNSS_CONFIGURATION_HPP_ */
