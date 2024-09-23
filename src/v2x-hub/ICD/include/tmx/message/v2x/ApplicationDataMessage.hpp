/*
 * ApplicationDataMessage.h
 *
 *  Created on: Jun 22, 2016
 *      Author: ivp
 */

#ifndef INCLUDE_APPLICATIONDATAMESSAGE_H_
#define INCLUDE_APPLICATIONDATAMESSAGE_H_

#include <tmx/plugin/dao/TmxDaoAttributes.hpp>

#include "ApplicationMessageEnumTypes.hpp"
#include "ApplicationDataMessageEnumTypes.hpp"

namespace tmx {
namespace message {
namespace v2x {

/**
 * ApplicationDataMessage is the message type used to send information messages about various application statuses.
 * It defines the message type and sub type and all data members.
 */
class ApplicationDataMessage {
    typedef ApplicationDataMessage self_type;

public:
    ApplicationDataMessage() noexcept = default;

    ApplicationDataMessage(std::string id, ApplicationTypes appId, //std::string eventID,
                           std::string timestamp, std::string interactionId, int intersectionId,
                           DataCodeId dataCode, std::string data) {
        set_Id(id);
        set_AppId(appId);
        //set_EventID(eventID);
        set_Timestamp(timestamp);
        set_InteractionId(interactionId);
        set_IntersectionId(intersectionId);
        set_DataCode(dataCode);

        set_Data(data);
    }

    ///unique guid
    tmx_dao_attribute(std::string, Id, "")

    ///Id of the application unique string
    tmx_dao_attribute(ApplicationTypes, AppId, ApplicationTypes::NOAPPID)

    ///Unique message identifier for repeated notification
    //tmx_dao_attribute(std::string, EventID, "")

    ///Timestamp of the event
    tmx_dao_attribute(std::string, Timestamp, "")

    //null/not present if not currently interacting with thing of interest.
    tmx_dao_attribute(std::string, InteractionId, "")

    //Intersection Id of the current map.  null/not present if not on a map.
    tmx_dao_attribute(int, IntersectionId, -2)

    ///Code from master list of possible types
    tmx_dao_attribute(DataCodeId, DataCode, DataCodeId::NOEVENTID)

    ///Json payload of data fields. Fields vary depending on DataCodeId.
    tmx_dao_attribute(std::string, Data, "")

};

} /* namespace v2x */
} /* namespace messages */
} /* namespace tmx */

#endif /* INCLUDE_APPLICATIONDATAMESSAGE_H_ */
