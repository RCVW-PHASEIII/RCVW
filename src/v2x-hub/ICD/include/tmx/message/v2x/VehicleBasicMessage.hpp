/*
 * VehicleBasicMessage.h
 *
 *  Created on: Apr 8, 2016
 *      Author: ivp
 */

#ifndef INCLUDE__VEHICLEBASICMESSAGE_H_
#define INCLUDE__VEHICLEBASICMESSAGE_H_

#include "VehicleParameterEnumTypes.hpp"

#include <tmx/plugin/dao/TmxDaoAttributes.hpp>

#define METERS_PER_MILE 1609.3435021075907
#define SECONDS_PER_HOUR 3600

namespace tmx {
namespace message {
namespace v2x {

/**
 * VehicleBasicMessage is the message type used to transmit information from the vehicle through TMX core.
 * It defines the message type and sub type and all data members.
 */
class VehicleBasicMessage {
    typedef VehicleBasicMessage self_type;

public:
    VehicleBasicMessage() = default;

    /// The gear shift position.
    tmx_dao_attribute(GearState, GearPosition, GearState::GearUnknown);

    /// Indicates whether the brake is currently applied.
    tmx_dao_attribute(GenericState, Brake, GenericState::Inactive)

    /// Indicates whether the anti-lock brake system is currently applied.
    tmx_dao_attribute(GenericState, ABS, GenericState::Inactive)

    /// Indicates whether the stability control system is currently applied.
    tmx_dao_attribute(GenericState, StabilityControl, GenericState::Inactive)

    /// The turn signal position.
    tmx_dao_attribute(TurnSignalState, TurnSignalPosition, TurnSignalState::SignalUnknown)

    /// The front door status
    tmx_dao_attribute(DoorState, FrontDoors, DoorState::Closed);

    /// The rear door status
    tmx_dao_attribute(DoorState, RearDoors, DoorState::Closed);

    /// The status of the head lights
    tmx_dao_attribute(GenericState, HeadLights, GenericState::Inactive);

    /// The status of the high beam headlights, i.e. the brights
    tmx_dao_attribute(GenericState, HighBeam, GenericState::Inactive);

    /// The status of the tail lights
    tmx_dao_attribute(GenericState, TailLights, GenericState::Inactive);

    /// The status of the brake lights
    tmx_dao_attribute(GenericState, BrakeLights, GenericState::Inactive);

    /// The status of the wipers
    tmx_dao_attribute(WiperState, Wipers, WiperState::WiperUnknown);

    /// The steering wheel angle
    tmx_dao_attribute(double, SteeringWheelAngle, 0.0);

    /// The accelerator pedal position
    tmx_dao_attribute(double, AcceleratorPosition, 0.0);

    /// The speed of the vehicle in meters per second
    tmx_dao_attribute(double, Speed, 0.0)

    /// The acceleration of the vehicle
    tmx_dao_attribute(double, Acceleration, 0.0);

    /// The speed of the left front wheelt
    tmx_dao_attribute(double, LeftFrontWheel, 0.0);

    /// The speed of the right front wheelt
    tmx_dao_attribute(double, RightFrontWheel, 0.0);

    /// The speed of the left front wheelt
    tmx_dao_attribute(double, LeftRearWheel, 0.0);

    /// The speed of the right front wheelt
    tmx_dao_attribute(double, RightRearWheel, 0.0);

    /// The length of the vehicle
    tmx_dao_attribute(double, VehicleLength, 0.0);

    /// The outside air temperature
    tmx_dao_attribute(double, Temp, 0.0);

    /**
     * This function is only for backwards compatibility
     * @deprecated
     */
    inline double get_Speed_mph() const {
        return this->get_Speed_mps() * (SECONDS_PER_HOUR / METERS_PER_MILE);
    }

    /**
     * This function is only for backwards compatibility
     * @deprecated
     */
    inline double get_Speed_mps() const {
        return this->get_Speed();
    }

    /**
     * This function is only for backwards compatibility
     * @deprecated
     */
    inline double get_Speed_kph() {
        return this->get_Speed() / 1000.0;
    }

    /**
     * This function is only for backwards compatibility
     * @deprecated
     */
    inline void set_Speed_mph(double mph) {
        this->set_Speed_mps(mph / (SECONDS_PER_HOUR / METERS_PER_MILE));
    }

    /**
     * This function is only for backwards compatibility
     * @deprecated
     */
    inline void set_Speed_mps(double mps) {
        this->set_Speed(mps);
    }

    /**
     * This function is only for backwards compatibility
     * @deprecated
     */
    inline void set_Speed_kph(double kph) {
        this->set_Speed_mps(kph / 1000.0);
    }
};

} /* namespace v2x */
} /* namespace messages */
} /* namespace tmx */

#endif /* INCLUDE__VEHICLEBASICMESSAGE_H_ */
