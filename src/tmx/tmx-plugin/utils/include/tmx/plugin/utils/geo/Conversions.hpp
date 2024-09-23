/*
 * Conversions.h
 *
 *  Created on: Oct 14, 2014
 *      Author: ivp
 */

#ifndef CONVERSIONS_H_
#define CONVERSIONS_H_

#include "WGS84Point.hpp"

namespace tmx {
namespace plugin {
namespace utils {
namespace geo {

class Conversions {
public:
    static double ConvertMetersToMiles(double meters);

    static double ConvertMilesToMeters(double miles);

    static double ConvertMetersPerSecToMilesPerHour(double mps);

    static double ConvertDegreesToRadians(double degrees);

    static double ConvertRadiansToDegrees(double radians);

    static int ConvertMetersPerSecToMPH(double mps);

    static double DistanceMeters(double degreesLat1, double degreesLon1, double degreesLat2, double degreesLon2);

    static double DistanceMeters(WGS84Point point1, WGS84Point point2);

    static double GetBearingDegrees(WGS84Point point1, WGS84Point point2);

    static double GradeDegrees(WGS84Point point1, WGS84Point point2);

    static double NodeOffsetToLatitude(double baseLatitude, double totalYOffsetMeters);

    static double NodeOffsetToLongitude(double baseLongitude, double baseLatitude, double totalXOffsetMeters);
};

}}}} // namespace tmx::plugin::utils::geo

#endif /* CONVERSIONS_H_ */
