/*
 * VehicleStateChange.h
 *
 *  Created on: Jun 10, 2016
 *      Author: ivp
 */

#ifndef INCLUDE_VEHICLESTATECHANGE_H_
#define INCLUDE_VEHICLESTATECHANGE_H_


#include <tmx/plugin/dao/TmxDaoAttributes.hpp>

#include "VehicleParameterEnumTypes.hpp"

namespace tmx {
namespace messages {
namespace v2x {

/**
 * ApplicationEventMessage is the message type used to send information messages about plugin status/activities.
 * It defines the message type and sub type and all data members.
 */
class VehicleStateChange {
    typedef VehicleStateChange self_type;

public:
    VehicleStateChange() = default;

    VehicleStateChange(std::string id, std::string timestamp, VehicleParameter vehicleParamId,
                       VehicleParameterState preState, VehicleParameterState postState) {
        set_Id(id);
        set_Timestamp(timestamp);
        set_VehicleParamId(vehicleParamId);
        set_PreState(preState);
        set_PostState(postState);
    }

    tmx_dao_attribute(std::string, Id, "")

    //Timestamp of the data change,
    tmx_dao_attribute(std::string, Timestamp, "")

    //Id of the Vehicle Parameter being reported
    tmx_dao_attribute(VehicleParameter, VehicleParamId, VehicleParameter::ParameterNA)

    //State of the parameter before the change,
    tmx_dao_attribute(VehicleParameterState, PreState, VehicleParameterState::StateNA)

    //State of the parameter after the change,
    tmx_dao_attribute(VehicleParameterState, PostState, VehicleParameterState::StateNA)
};

} /* namespace v2x */
} /* namespace messages */
} /* namespace tmx */


#endif /* INCLUDE_VEHICLESTATECHANGE_H_ */
