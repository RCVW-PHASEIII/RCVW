/*
 * InpixionSwarm.h
 *
 *  Created on: May 5, 2022
 *      Author: gmb
 */

#ifndef INCLUDE_INPIXIONSWARM_H_
#define INCLUDE_INPIXIONSWARM_H_

#include <tmx/messages/message.hpp>

#include <Measurement.h>
#include "MessageTypes.h"
#include <vector>

namespace tmx {
namespace messages {


class SwarmRangeMessage: public tmx::message {
	typedef Measurement<units::Distance, units::Distance::m> mMeas;
	typedef Measurement<units::Acceleration, units::Acceleration::mperspers> aMeas;
	typedef Measurement<units::Power, units::Power::dBm> dbMeas;
	typedef Measurement<units::Percent, units::Percent::pct> pctMeas;

public:
	SwarmRangeMessage(): tmx::message() { }

	static constexpr const char *MessageType = "Range";
	static constexpr const char *MessageSubType = "Swarm";

	std_attribute(this->msg, std::string, from, "", )
	std_attribute(this->msg, std::string, to, "", )
	std_attribute(this->msg, mMeas, range, 0.0, )
	std_attribute(this->msg, uint16_t, device_class, 0, )
	std_attribute(this->msg, aMeas, x_accel, 0.0, )
	std_attribute(this->msg, aMeas, y_accel, 0.0, )
	std_attribute(this->msg, aMeas, z_accel, 0.0, )
	std_attribute(this->msg, dbMeas, rssi, 0.0, )
	std_attribute(this->msg, pctMeas, battery, 0.0, )
	std_attribute(this->msg, uint64_t, timestamp, 0, )
	std_attribute(this->msg, uint64_t, time, 0, )
};

class SwarmAPIMessage: public tmx::message {
public:
	SwarmAPIMessage(): tmx::message() { }

	static constexpr const char *MessageType = MSGTYPE_DECODED_STRING;
	static constexpr const char *MessageSubType = "Swarm";

	std::vector< SwarmRangeMessage > get_messages() {
		std::vector< SwarmRangeMessage > messages;
		auto &tree = *(this->as_tree());
		for (auto &key: tree) {
			messages.emplace_back();
			auto &msg = messages[messages.size() - 1];

			msg.set_contents(key.second);
			msg.set_from(key.first);
		}
		return messages;
	}
};


} /* namespace messages */
} /* namespace tmx */



#endif /* INCLUDE_INPIXIONSWARM_H_ */
