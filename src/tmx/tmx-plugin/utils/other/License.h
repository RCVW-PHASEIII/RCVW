/*
 * License.h
 *
 *  Created on: Nov 8, 2018
 *      @author: Gregory M. Baumgardner
 */

#ifndef SRC_LICENSE_H_
#define SRC_LICENSE_H_

#include <tmx/messages/byte_stream.hpp>
#include <tmx/messages/message.hpp>

#include "Clock.h"

namespace tmx {
namespace utils {

/**
 * This class represents a simple feature locking license implementation for TMX software.
 *
 * It is represented here by a JSON message container with information about the organization,
 * the time the license was generated, and an encrypted key that contains the feature set for this license.
 *
 * Some basic constraints:
 *  -	There are default features enabled that require no valid key, which will be the basic V2I Hub capabilities
 * 	-	A decoded feature set stored in the license file is ignored
 * 	-	The license can expire after some number of days, which if left unspecified means no expiration
 * 	-	Changes to the organization or timestamp or expiring days will render the license useless
 */
class License: public tmx::message {
public:
	static constexpr const char *MessageType = "License";
	static constexpr const char *MessageSubType = "License";

	License();
	virtual ~License();

	/**
	 * The organization information, as a URL
	 */
	ro_attribute(this->msg, battelle::attributes::standard_attribute<organization>, std::string, organization, get_, "");

	/**
	 * The encrypted license key as a string of bytes
	 */
	ro_attribute(this->msg, battelle::attributes::standard_attribute<key>, std::string, key, get_, "");

	/**
	 * The timestamp of the license key
	 */
	ro_attribute(this->msg, battelle::attributes::standard_attribute<timestamp>, uint64_t, timestamp, get_, Clock::GetMillisecondsSinceEpoch());

	/**
	 * The number of days the license key is good for.  Any value under 1 means unlimited.
	 */
	ro_attribute(this->msg, battelle::attributes::standard_attribute<days>, short, days, get_, 0);

	/**
	 * @return True if the key has been correctly decoded.  False, otherwise.
	 */
	bool isKeyValid();

	/**
	 * @return True if the license key has expired.  False otherwise.
	 */
	bool isExpired();

	/**
	 * @return True if the specified feature is enabled.  False otherwise.
	 * @param feature The name of the TMX licensed feature
	 */
	bool isEnabled(std::string feature);
};


} /* namespace utils */
} /* namespace tmx */

#endif /* SRC_LICENSE_H_ */
