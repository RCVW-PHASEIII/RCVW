/*
 * LocationMessage.h
 *
 *  Created on: Apr 21, 2016
 *      Author: ivp
 */

#ifndef INCLUDE_LOCATIONMESSAGE_H_
#define INCLUDE_LOCATIONMESSAGE_H_

#include <tmx/common/types/Any.hpp>
#include <tmx/plugin/dao/TmxDaoAttributes.hpp>

#include "LocationMessageEnumTypes.hpp"

#define METERS_PER_MILE 1609.3435021075907
#define SECONDS_PER_HOUR 3600

namespace tmx {
namespace message {
namespace v2x {

/**
 * LocationMessage is the message type used to send information messages about plugin status/activities.
 * It defines the message type and sub type and all data members.
 */
class LocationMessage {
    typedef LocationMessage self_type;

public:
    LocationMessage() { }

    LocationMessage(const LocationMessage &copy) :
            _Id(copy._Id), _SignalQuality(copy._SignalQuality), _SentenceIdentifier(copy._SentenceIdentifier),
            _Time(copy._Time), _Latitude(copy._Latitude), _Longitude(copy._Longitude), _Altitude(copy._Altitude),
            _FixQuality(copy._FixQuality), _NumSatellites(copy._NumSatellites), _HorizontalDOP(copy._HorizontalDOP),
            _Speed(copy._Speed), _Heading(copy._Heading) { }

    LocationMessage(std::string id, SignalQualityTypes signalQuality, std::string sentenceIdentifier,
                    uint64_t time,
                    double latitude, double longitude, FixTypes fixQuality, int numSatellites,
                    double horizontalDOP, double speed, double heading) {
        set_Id(id);
        set_SignalQuality(signalQuality);
        set_SentenceIdentifier(sentenceIdentifier);
        set_Time(time);
        set_Latitude(latitude);
        set_Longitude(longitude);
        set_FixQuality(fixQuality);
        set_NumSatellites(numSatellites);
        set_HorizontalDOP(horizontalDOP);
        set_Speed_mps(speed);
        set_Heading(heading);
    }

    LocationMessage(message::TmxData const &value) {
        static std::tuple<Id_attr, SignalQuality_attr, SentenceIdentifier_attr,
                Time_attr, Latitude_attr, Longitude_attr, FixQuality_attr,
                NumSatellites_attr, HorizontalDOP_attr, Altitude_attr,
                Speed_attr, Heading_attr> _tuple{ };
        //TLOG(INFO) << std::this_thread::get_id() << ": Enter " << TMX_PRETTY_FUNCTION;
        //_Id = value[Id_s.c_str()];
        //_SignalQuality = (location::SignalQualityTypes)value[SignalQuality_s.c_str()];
        //_SentenceIdentifier = value[SentenceIdentifier_s.c_str()];
        //_Time = value[Time_s.c_str()];
        //_Latitude = value[Latitude_s.c_str()];
        //_Longitude = value[Longitude_s.c_str()];
        //_FixQuality = (location::FixTypes)value[FixQuality_s.c_str()];
        //_NumSatellites = value[NumSatellites_s.c_str()];
        //_HorizontalDOP = value[HorizontalDOP_s.c_str()];
        //_Altitude = value[Altitude_s.c_str()];
//        _Speed = value[Speed_s.c_str()];
        //_Heading = value[Heading_s.c_str()];
        //TLOG(INFO) << std::this_thread::get_id() << ": Exit " << TMX_PRETTY_FUNCTION;

        tie_attributes(value, _tuple, _Id, _SignalQuality, _SentenceIdentifier,
                       _Time, _Latitude, _Longitude, _FixQuality,
                       _NumSatellites, _HorizontalDOP, _Altitude,
                       _Speed, _Heading);
    }

tmx_dao_attribute(std::string, Id, "")

tmx_dao_attribute(SignalQualityTypes, SignalQuality, SignalQualityTypes::Invalid)
    /**
         * $GPGGA Global Positioning System Fix Data.Time, position and fix related data for a GPS receiver.
         */
tmx_dao_attribute(std::string, SentenceIdentifier, "")
    /**
     * hhmmss.ss = UTC of position. (ex: 170834	        is  17:08:34 Z)
     */
tmx_dao_attribute(uint64_t, Time, 0)
    /**
     * llll.ll = latitude of position (ex: 4124.8963, N        is 	41d 24.8963' N or 41d 24' 54" N)
     * 	a = N or S
     */
tmx_dao_attribute(double, Latitude, 0.0)
    /**
     * 	yyyyy.yy = Longitude of position (ex: 08151.6838, W        is 81d 51.6838' W or 81d 51' 41" W)
     * a = E or W
     */
tmx_dao_attribute(double, Longitude, 0.0)
    /**
     * 	x = GPS Quality indicator (0=no fix, 1=GPS fix, 2=Dif. GPS fix)
     */
tmx_dao_attribute(FixTypes, FixQuality, FixTypes::Unknown)
    /**
     * 	xx = number of satellites in use (ex: 	05	is 5 Satellites are in view)
     */
tmx_dao_attribute(int, NumSatellites, 0)
    /**
     * 	x.x = horizontal dilution of precision (ex: 1.5	is Relative accuracy of horizontal position)
     */
tmx_dao_attribute(double, HorizontalDOP, 0)
    /**
     * 	x.x = Antenna altitude above mean-sea-level (ex: 280.2, M	is   280.2 meters above mean sea level)
M = units of antenna altitude, meters
     */
tmx_dao_attribute(double, Altitude, 0.0)
    /**
     * x.x = Geoidal separation  - Height of geoid above WGS84 ellipsoid.  (ex: -34.0, M	is   -34.0 meters)
M = units of geoidal separation, meters
     */
    //std_attribute(this->msg,double, GeoidalSeparation, 0, )
    /**
     * Time since last DGPS update.
     * x.x = Age of Differential GPS data (seconds)
     */
    //std_attribute(this->msg,int, SecSinceLastUpdate, 0, )
    /**
     * DGPS reference station id
     * xxxx = Differential reference station ID
     */
    //std_attribute(this->msg,std:string, RefStationId, 0, )
    /**
     * Checksum. Used by program to check for transmission errors.
     */
    //std_attribute(this->msg,int, Checksum, 0, )

    /**
     * x.x,K = Speed, m/s
     *  (ex: 010.2,K      Ground speed, meters per second)
     */
tmx_dao_attribute(double, Speed, 0.0)

    /**
     * Heading in degrees.
     */
tmx_dao_attribute(double, Heading, 0.0)


    //eg2. $--GGA,hhmmss.ss,llll.ll,a,yyyyy.yy,a,x,xx,x.x,x.x,M,x.x,M,x.x,xxxx

    /**
	 * This function is only for backwards compatibility
	 * @deprecated
	 */
    inline double get_Speed_mph() const noexcept {
        return this->get_Speed_mps() * (SECONDS_PER_HOUR / METERS_PER_MILE);
    }

    /**
     * This function is only for backwards compatibility
     * @deprecated
     */
    inline double get_Speed_mps() const noexcept {
        return this->get_Speed();
    }

    /**
     * This function is only for backwards compatibility
     * @deprecated
     */
    inline double get_Speed_kph() const noexcept {
        return this->get_Speed() / 1000.0;
    }

    /**
     * This function is only for backwards compatibility
     * @deprecated
     */
    inline void set_Speed_mph(double mph) noexcept {
        this->set_Speed_mps(mph / (SECONDS_PER_HOUR / METERS_PER_MILE));
    }

    /**
     * This function is only for backwards compatibility
     * @deprecated
     */
    inline void set_Speed_mps(double mps) noexcept {
        this->set_Speed(mps);
    }

    /**
     * This function is only for backwards compatibility
     * @deprecated
     */
    inline void set_Speed_kph(double kph) noexcept {
        this->set_Speed_mps(kph / 1000.0);
    }

};

} /* namespace v2x */
} /* namespace message */
} /* namespace tmx */

#endif /* INCLUDE_LOCATIONMESSAGE_H_ */
