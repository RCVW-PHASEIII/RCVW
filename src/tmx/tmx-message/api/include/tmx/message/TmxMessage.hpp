/*!
 * Copyright (c) 2021 Battelle Memorial Institute
 *
 * All Rights Reserved.
 *
 * @file TmxMessage.hpp
 *
 *  Created on: Jun 16, 2021
 *      @author: Gregory M. Baumgardner
 */

#ifndef INCLUDE_TMX_MESSAGE_TMXMESSAGE_HPP_
#define INCLUDE_TMX_MESSAGE_TMXMESSAGE_HPP_

#include <tmx/common/types/bytes_type.hpp>
#include <tmx/common/types/int_type.hpp>
#include <tmx/common/types/string_type.hpp>

#include <chrono>
#include <cinttypes>
#include <string_view>

namespace tmx {
namespace message {

class TmxMessage {
public:
	using string_type = tmx::common::types::string;
	using int_type 	  = tmx::common::types::uint64;

	typedef tmx::common::types::bytes_type<> bytes_t;
	typedef typename bytes_t::byte_type byte_t;

	using payload_type = tmx::common::types::byte_string<byte_t>;

	TmxMessage();
	virtual ~TmxMessage();
	TmxMessage(const TmxMessage &other);
	TmxMessage(TmxMessage &&other);

	TmxMessage& operator=(const TmxMessage &other);
	TmxMessage& operator=(TmxMessage &&other);

	// The following methods provide direct access to the members

	/*!
	 * @return The message type identifier, to read or write data
	 */
	inline auto &type_id() { return this->_type_id; }
	inline auto type_id() const { return this->_type_id; }

	/*!
	 * @return The message topic, to read or write data
	 */
	inline auto &topic() { return this->_topic; }
	inline auto topic() const { return this->_topic; }

	/*!
	 * @return The message source, to read or write data
	 */
	inline auto &source() { return this->_source; }
	inline auto source() const { return this->_source; }

	/*!
	 * @return The encoding, to read or write data
	 */
	inline auto &encoding() { return this->_encoding; }
	inline auto encoding() const { return this->_encoding; }

	/*!
	 * @return The metadata, to read or write data
	 */
	inline auto &metadata() { return this->_metadata; }
	inline auto metadata() const { return this->_metadata; }

	// The following methods are getters and setters for the less-accessible data contents
	/*!
	 * @return The timestamp as an integer value
	 */
	inline auto get_timestamp() const { return this->_timestamp; }

	/*!
	 * Get the timestamp as the C++ standard time point class. Note that the underlying
	 * time point representation and the duration size and count is determined by the
	 * compiler based on the target system. In most cases, this should be similar to the
	 * default Linux case with a nanosecond count since the epoch. However, the template
	 * parameters may be modified if the specific use case demands it.
	 *
	 * @return The point in time represented by this timestamp.
	 */
	template <class Clock = std::chrono::system_clock, class Duration = typename Clock::duration>
	inline auto get_timepoint() const {
		return std::chrono::time_point<Clock, Duration>(
				static_cast<typename Duration::Rep>(this->get_timestamp()));
	}

	/*!
	 * Set the timestamp by numeric value.
	 *
	 * @param[in] timestamp The timestamp value
	 */
	template <typename T>
	inline void set_timestamp(T timestamp) {
		this->_timestamp = static_cast<decltype(this->_timestamp)>(timestamp);
	}

	/*!
	 * Set the timestamp from the given C++ standard time point class. Note that the
	 * underlying time point representation and the duration size and count is determined
	 * by the compiler based on the target system. In most cases, this should be similar
	 * to the default Linux case with a nanosecond count since the epoch. However, the
	 * template parameters may be modified if the specific use case demands it.
	 *
	 * @param[in[ timepoint The timepoint to use
	 */
	template <class Clock = std::chrono::system_clock, class Duration = typename Clock::duration>
	inline auto set_timepoint(const std::chrono::time_point<Clock, Duration> &timepoint) {
		this->set_timestamp(timepoint.time_since_epoch().count());
	}

	/*!
	 * @return The number of bytes in the payload
	 */
	typename bytes_t::size_type get_length() const;

	/*!
	 * @return The sequence of payload bytes
	 */
	bytes_t get_payload() const;

	/*!
	 * @brief Set the payload from the given sequence of bytes.
	 *
	 * @param[in] bytes A pointer to the byte sequence
	 * @param[in] size The number of bytes in the sequence
	 */
	void set_payload(bytes_t bytes);

private:
	/*!
	 * The message type identifier, as text. This field denotes a fully-qualified
	 * identification of the message contained. This field is required, but is typically
	 * set automatically through introspection of the message container used. The type
	 * identifier name separator is typically that of a C++ class, namely "::", but can
	 * be anything as long as it is known at the time in which the payload decoding is done.
	 *
	 * Previous TMX versions use a “type” specifier, such as “J2735” for J2735 encoded
	 * types, plus a “subtype” identifier for the type of J2735 message, such as “BSM”
	 * for the Basic Safety Message. However, now this must be handled explicitly through
	 * namespace identification, for example "tmx::message::J2735::BSM" could represent
	 * the type of "J2735" and sub-type of "BSM", and therefore the simplest solution
	 * is construct a message container class of name "BSM" in the "tmx::message::J2735"
	 * namespace.
	 */
	string_type _type_id;

	/*!
	 * The message topic, as text. Borrowing from existing message broker frameworks, this field
	 * denotes the topic, or channel, that message is to be sent or was received from. Conceptually,
	 * this should follow standardized MQTT guidelines for topic names, such as using a
	 * “/” separator, but there is no requirement that enforces such, in order to ensure
	 * compatability with any supported brokers.
	 */
	string_type _topic;

	/*!
	 * An identifier of the message source, as text. This denotes the entity, such as a TMX plugin
	 * or an external component, where the message was originally created. Since this field is
	 * mainly for informational purposes, it is therefore optional and unspecified values shall
	 * be listed as an empty string, i.e., “”.
	 */
	string_type _source;

	/*!
	 * The type of encoding to use for the payload, as text. This field is optional, and unspecified
	 * values shall be listed as an empty string, i.e., “”. However, the TMX system will automatically
	 * set this value upon encoding based on the type of data discovered inside the message container.
	 * If this field is found to be empty when decoding is attemped, then the default encoding is
	 * assumed based on the leading characters in the payload. For example, if a { is detected, then
	 * the payload is assumed to be JSON-encoded. As a last resort, the message payload is assumed to
	 * be a basic string whose contents are the supplied bytes.
	 */
	string_type _encoding;

	/*!
	 * The time at which the message was created, stored as the count of some time unit duration.
	 * Typically, this is assumed to be the number of nanoseconds since the epoch. However, specialized
	 * plugins can communicate any count required, provided that both ends can determine what that
	 * count represents. For example, since the biggest value that this 64-bit field can store is
	 * 18,446,744,073,709,551,615, the maximum timestamp that can be represented by nanosecond count is:
	 * Sun Jul 21 19:34:33 EDT 2554
	 *
	 * Therefore, in theory, the program could use the count of the number of microseconds since
	 * the epoch, which would extend the timeframe to the year 586524. A more rational example would
	 * might be counting the number of nanoseconds from the start of a plugin. In either case, if
	 * TMX system logs the timestamp as a human-readble date text, the altered scale would cause this
	 * information to appear incorrect.
	 *
	 * Best effort is given to carrying this time throughout transmission, but this is not guaranteed
	 * to be the case.
	 */
	int_type 		 _timestamp		{ 0 };

	/*!
	 * This 8-bytes of information is used by routing mechanisms and/or downstream plug-ins to provide
	 * higher-level metadata on the contents of the messages. For example, QoS data may be specified
	 * in the upper-order 2 bits, such as 00 for QoS0, 01 for QoS1, 10 for QoS2 or 11 for QoSX, i.e.,
	 * any quality of service. Likewise, the second byte may store DSRC channel to broadcast to,
	 * such as 0xB6 for channel 182 (5.910 GHz) if the message is to be sent out a radio. Other use
	 * cases exists, including specifying a timestamp duration count unit, but these must be
	 * interpreted independently by each plugin that requires this detail. The low-order byte, i.e.,
	 * bits 0-7, shall be reserved for the fragmentation of message packets, if it is required
	 * by the broker.
	 */
	int_type		 _metadata		{ 0 };

	/*!
	 * The actual binary bytes to send in the message, which varies significantly by the application.
	 * Note that for clarity, the payload is always stored as a sequence of unsigned characters, i.e.,
	 * plain old bytes, instead of the signed type used in character strings. Consideration should be
	 * given to the total size of the message, including the payload and all the header information,
	 * such as topic and encoding. While most messaging broker frameworks would be able to route a
	 * reasonably sized packet, such as 4KB or less, it is always possible that transmission gets
	 * garbled between multiple messages being received at the same time or fragmented to some PDU of
	 * the networking interface. If buffering is required, the broker implementation must specifically
	 * handle it.  There is room in the message framework for a specific broker implementation to
	 * split and reassemble individual messages into pieces, if necessary, but it must be done
	 * programmatically by breaking the single message into separate messages of smaller lengths, with
	 * ordering specified in the metadata. Note that only a single, and complete, message shall be passed
	 * into or returned from the broker API.
	 */
	payload_type	_payload;
};



} /* End namespace message */
} /* End namespace message */

#endif /* INCLUDE_TMX_MESSAGE_TMXMESSAGE_HPP_ */
