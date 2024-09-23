/*
 * ApplicationMessage.h
 *
 *  Created on: Jun 7, 2016
 *      Author: ivp
 */

#ifndef INCLUDE_APPLICATIONMESSAGE_H_
#define INCLUDE_APPLICATIONMESSAGE_H_

#include <tmx/plugin/dao/TmxDaoAttributes.hpp>

#include "ApplicationMessageEnumTypes.hpp"

namespace tmx {
namespace message {
namespace v2x {

/**
 * ApplicationMessage is the message type used to send information messages about various application statuses.
 * It defines the message type and sub type and all data members.
 */
class ApplicationMessage {
    typedef ApplicationMessage self_type;

public:
    ApplicationMessage() noexcept = default;

    ApplicationMessage(std::string id, ApplicationTypes appId, std::string eventID,
                       std::string timestamp, std::string displayDuration, Severity severity,
                       EventCodeTypes eventCode, std::string interactionId, std::string customText) {
        set_Id(id);
        set_AppId(appId);
        set_EventID(eventID);
        set_Timestamp(timestamp);
        set_DisplayDuration(displayDuration);
        set_Severity(severity);
        set_EventCode(eventCode);
        set_InteractionId(interactionId);
        set_CustomText(customText);
    }

    ApplicationMessage(std::string id, ApplicationTypes appId, std::string eventID,
                       std::string timestamp, std::string displayDuration, Severity severity,
                       EventCodeTypes eventCode, std::string interactionId, std::string customText,
                       double distanceToRefPoint, double angleToRefPoint) {
        set_Id(id);
        set_AppId(appId);
        set_EventID(eventID);
        set_Timestamp(timestamp);
        set_DisplayDuration(displayDuration);
        set_Severity(severity);
        set_EventCode(eventCode);
        set_InteractionId(interactionId);
        set_CustomText(customText);
        set_DistanceToRefPoint(distanceToRefPoint);
        set_AngleToRefPoint(angleToRefPoint);
    }

    ApplicationMessage(message::TmxData const &value) {
        static std::tuple<Id_attr, AppId_attr, EventID_attr, Timestamp_attr,
                          DisplayDuration_attr, Severity_attr, EventCode_attr,
                          InteractionId_attr, CustomText_attr , DistanceToRefPoint_attr,
                          AngleToRefPoint_attr> _tuple { };
        tie_attributes(value.get_container(), _tuple, _Id, _AppId, _EventID, _Timestamp,
                       _DisplayDuration, _Severity, _EventCode,
                       _InteractionId, _CustomText, _DistanceToRefPoint,
                       _AngleToRefPoint);
    }

    //unique guid
    tmx_dao_attribute(std::string, Id, "")

    //Id of the application unique string
    tmx_dao_attribute(ApplicationTypes, AppId, ApplicationTypes::NOAPPID)

    //Unique message identifier for repeated notification
    tmx_dao_attribute(std::string, EventID, "")

    //Timestamp of the event
    tmx_dao_attribute(std::string, Timestamp, "")

    //How long to display in milliseconds
    tmx_dao_attribute(std::string, DisplayDuration, "")

    //Info or InformAlert or WarnAler
    tmx_dao_attribute(Severity, Severity, Severity::Info)

    //Code from master list of possible events
    tmx_dao_attribute(EventCodeTypes, EventCode, EventCodeTypes::NOEVENTID)

    //null/not present if not currently interacting with thing of interest.
    tmx_dao_attribute(std::string, InteractionId, "")

    //Open text field for specific messages related to this event
    tmx_dao_attribute(std::string, CustomText, "")

    // Distance to the Reference point of the Map
    tmx_dao_attribute(double, DistanceToRefPoint, 0)

    //Open text field for specific messages related to this event
    tmx_dao_attribute(double, AngleToRefPoint, 0)
};

} /* namespace v2x */
} /* namespace messages */
} /* namespace tmx */

#endif /* INCLUDE_APPLICATIONMESSAGE_H_ */
