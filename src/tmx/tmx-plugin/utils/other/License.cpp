/*
 * License.cpp
 *
 *  Created on: Nov 8, 2018
 *      Author: gmb
 */

#include "License.h"

#ifdef TMX_LICENSE_KEY_DEBUG
#include <PluginLog.h>
#endif

#include "AES.h"

#include <cstring>
#include <openssl/sha.h>
#include <uuid/uuid.h>

using namespace std;
using namespace tmx;

namespace tmx {
namespace utils {

byte_stream generate_decoder_pin(License &lic) {
	// The feature set is encrypted with a combination of 3 keys, each of which utilize a UUID key:
	// 1. The timestamp encoded as v1 UUID, with a clock based on expire days and 00:00:00:00:00:00 MAC
	// 2. The organization URL encoded as v5 (SHA1) UUID
	// 3. A secret Battelle URL encoded as v3 (SHA1) UUID
	//
	// The resulting 3 128-byte words are combined together as a re-ordered set of bytes that are
	// XOR'd together

	uuid_t timeUUID;
	uuid_t orgUUID;
	uuid_t secretUUID;
	uuid_t keyUUID;

	// The time UUID is based on the timestamp, which is a count of ms since the epoch.  The UUID
	// timestamp is a count of 100 ns periods since Oct 15 1528, when the Gregorian calendar was founded.
	// See https://tools.ietf.org/html/rfc4122
	memset(timeUUID, 0, sizeof(uuid_t));
	uint64_t time = lic.get_timestamp() + 12219292800000;
	time *= 10000;	// ms to 100 ns conversion

	// Set the lower order 32 bits
	timeUUID[3] = (time >>  0) & 0xFF;
	timeUUID[2] = (time >>  8) & 0xFF;
	timeUUID[1] = (time >> 16) & 0xFF;
	timeUUID[0] = (time >> 24) & 0xFF;

	// Set the middle order 2 bytes
	timeUUID[5] = (time >> 32) & 0xFF;
	timeUUID[4] = (time >> 40) & 0xFF;

	// Set the highest order 1.5 bytes
	timeUUID[7] = (time >> 48) & 0xFF;
	timeUUID[6] = (time >> 56) & 0x0F;

	// Set the version numbers
	timeUUID[6] |= 0x10;
	timeUUID[8] |= 0x80;

	// Set the days to expire in 12 bits
	short d = lic.get_days();
	timeUUID[8] |= (d >> 8) & 0x0F;
	timeUUID[9] |= (d >> 0) & 0xFF;


#ifdef TMX_LICENSE_KEY_DEBUG
	PLOG(logDEBUG) << "Time UUID=" << byte_stream(timeUUID, timeUUID + sizeof(uuid_t));
#endif
	struct timeval tv;
	uuid_time(timeUUID, &tv);

	uint64_t timeCheck = tv.tv_sec * 1000 + tv.tv_usec / 1000;
	if (timeCheck != lic.get_timestamp())
		return byte_stream_decode("");

	// The organizational URL is used to encode a second UUID
	SHA_CTX ctx;
	byte_stream hashBytes(SHA_DIGEST_LENGTH);

	uuid_parse("6ba7b811-9dad-11d1-80b4-00c04fd430c8", orgUUID);

    SHA1_Init(&ctx);
    SHA1_Update(&ctx, &orgUUID, sizeof(uuid_t));
    SHA1_Update(&ctx, lic.get_organization().c_str(), lic.get_organization().size());
    SHA1_Final(hashBytes.data(), &ctx);

    memcpy(orgUUID, hashBytes.data(), sizeof(uuid_t));
    orgUUID[6] &= 0x0F;
    orgUUID[6] |= 0x50;
    orgUUID[8] &= 0x3F;
    orgUUID[8] |= 0x80;

#ifdef TMX_LICENSE_KEY_DEBUG
	PLOG(logDEBUG) << "Org UUID=" << byte_stream(orgUUID, orgUUID + sizeof(uuid_t));
#endif

	// Secret URL is http://www.battelle.org/ci/tbl/tmx/B@ttelle
    // That v3 UUID is 243f3d97-805b-322b-8b0e-d13517828e27 (SIV 48180571780507358399216727796643696167)
    // Not directly assigning the strings in order to hide the ASCII symbols in the debug code
    uint32_t u1 = 0x243f3d97;
    uint16_t u2 = 0x805b;
    uint16_t u3 = 0x322b;
    uint16_t u4 = 0x8b0e;
    uint64_t u5 = 0xd13517828e27;

    char uuidStr[128];
    sprintf(uuidStr, "%x-%x-%x-%x-%lx", u1, u2, u3, u4, u5);
    uuid_parse(uuidStr, secretUUID);

#ifdef TMX_LICENSE_KEY_DEBUG
	PLOG(logDEBUG) << "Secret UUID=" << byte_stream(secretUUID, secretUUID + sizeof(uuid_t));
#endif

    // Mix and match combinations for the key UUID
    const uint16_t tIdx[] = { 2, 8, 15, 4, 1, 7, 11, 3, 12, 9, 13, 0, 5, 6, 14, 10 };
    const uint16_t oIdx[] = { 11, 5, 0, 6, 10, 2, 9, 14, 8, 1, 15, 3, 12, 7, 13, 4 };
    const uint16_t sIdx[] = { 15, 14, 13, 12, 11, 10, 0, 1, 2, 3, 9, 8, 7, 6, 5, 4 };

    for (size_t i = 0; i < sizeof(uuid_t); i++)
    	keyUUID[i] = timeUUID[tIdx[i]] ^ orgUUID[oIdx[i]] ^ secretUUID[sIdx[i]];

#ifdef TMX_LICENSE_KEY_DEBUG
	PLOG(logDEBUG) << "Decoder UUID=" << byte_stream(keyUUID, keyUUID + sizeof(uuid_t));
#endif

    return byte_stream(keyUUID, keyUUID + sizeof(uuid_t));
}

License::License(): tmx::message() { }
License::~License() { }

bool License::isKeyValid() {
	static int msgVer = -50;
	static bool valid = false;

	if (msgVer < 0 || msgVer != this->msg.get_storage_version()) {
		AES aes(generate_decoder_pin(*this));

		// Clear out any old feature sets
		boost::optional<message_tree_type &> fTree = this->msg.get_storage().get_tree().get_child_optional("features");
		if (fTree)
			fTree.get().clear();

		byte_stream bytes = aes.decrypt(byte_stream_decode(this->get_key()));
		string fSet((const char *)bytes.data(), bytes.size());

		// Check that the key decoded ok
		valid = (fSet.length() >= 2 && fSet.substr(0, 1) == "{" && string(fSet.end() - 1, fSet.end()) == "}");

		if (valid) {
			// Insert the feature set to the tree
			message fMsg;
			fMsg.set_contents(fSet);

			message_tree_type subTree(fMsg.get_container().get_storage().get_tree());
			this->as_tree().get().put_child("features", subTree);
		}

		msgVer = this->msg.get_storage_version();
	}

	return valid;
}

bool License::isExpired() {
	// Only check the days if the key is valid
	if (this->isKeyValid()) {
		chrono::system_clock::time_point tp = Clock::GetTimepointSinceEpoch(this->get_timestamp());

		chrono::hours diffHrs = chrono::duration_cast<chrono::hours>(chrono::system_clock::now() - tp);
		return this->get_days() > 0 && this->get_days() < diffHrs.count() / 24;
	}

	// If key is not valid, then assume no expiring
	return false;
}

bool License::isEnabled(string feature) {
	// The key has to be valid and not expired
	return isKeyValid() && !isExpired() && this->as_tree().get().template get<bool>("features." + feature, false);
}

} /* namespace utils */
} /* namespace tmx */
