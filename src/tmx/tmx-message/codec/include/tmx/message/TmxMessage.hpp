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

#define TMX_METADATA_QOS_BITS 2
#define TMX_METADATA_PRIORITY_BITS 4
#define TMX_METADATA_BASE_BITS 2
#define TMX_METADATA_ASSIGNMENT_GROUP_BITS 4
#define TMX_METADATA_ASSIGNMENT_ID_BITS 4
#define TMX_METADATA_FRAGMENT_BITS 4
#define TMX_METADATA_ATTEMPT_BITS 4

#include <tmx/platform.hpp>

#include <tmx/message/TmxData.hpp>

#include <bitset>
#include <chrono>
#include <cinttypes>
#include <string_view>

namespace tmx {
namespace message {

/*!
 * @brief A class to hold the TMX message data elements
 */
class TmxMessage: public TmxData {
    typedef tmx::common::char_t char_t;
    typedef tmx::common::byte_t byte_t;
    typedef std::basic_string<char_t> string_type;
    typedef typename common::types::Intmax::value_type int_type;

public:
    TmxMessage() noexcept;
	TmxMessage(const TmxMessage &) noexcept;
    TmxMessage(TmxData &&) noexcept;
	virtual ~TmxMessage() = default;

	TmxMessage& operator=(TmxMessage const &) noexcept;
    TmxMessage& operator=(TmxData const &) noexcept;

    /*!
     * Many messaging systems use some synchronization bytes in order to identify the
     * start of the message or similar reasons. This becomes particularly important in
     * the case of variable length encodings, such as those in the ITU X.600 specifications
     * in order to ensure that the correct data type is encoded. For example, the SAE J2735
     * Message Frame type includes an identifier in the first two bytes to help the programmer
     * understand the content of the incoming data. Therefore, in order to best utilize
     * variable length encodings for message passage and thus increase overall throughput,
     * a TMX message should have an easily identifiable preamble.
     *
     * Since the whole purpose of TMX™ is to manage encoded data, it makes sense that the
     * preamble just be an encoded version of the name TMX. Using a 0-based count of letters
     * from A, the acronym TMX can be reduced to (19)(12)(23), or (0x13)(0x0C)(0x17), which
     * can further be encoded as 3 packed 5-bit integers resulting in two bytes: 0x4D97
     *
     * @return The fixed preamble for a TMX message, to be added externally to the container only if needed
     */
    static TMX_CONSTEXPR_FN typename common::types::UInt16::value_type get_preamble() {
        return 0x4D97;
    }

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
     *
	 * @return The message type identifier
	 */
    string_type const &get_id() const noexcept;


    /*!
     * This function returns a reference to facilitate copy-free usage or auto-update
     *
     * @return The message type identifier
     */
    string_type &get_id() noexcept;

    /*!
     * @return The type descriptor referenced by the message type identifier
     */
    common::TmxTypeDescriptor get_type() const noexcept;

    /*!
     * @brief Set the message type identifier
     *
     * This function copies the given string
     *
     * @param[in] The message type identifier
     */
    void set_id(string_type const &) noexcept;

	/*!
	 * The message topic, as text. Borrowing from existing message broker frameworks, this field
	 * denotes the topic, or channel, that message is to be sent or was received from. Conceptually,
	 * this should follow standardized MQTT guidelines for topic names, such as using a
	 * “/” separator, but there is no requirement that enforces such, in order to ensure
	 * compatability with any supported brokers.
	 *
	 * @return The message topic
	 */
    string_type const &get_topic() const noexcept;

    /*!
     * This function returns a reference to facilitate copy-free usage or auto-update
     *
     * @return The message topic
     */
    string_type &get_topic() noexcept;

    /*!
     * @brief Set the message topic
     *
     * This function copies the given string
     *
     * @param[in] The message topic
     */
    void set_topic(string_type const &) noexcept;

	/*!
	 * An identifier of the message source, as text. This denotes the entity, such as a TMX plugin
	 * or an external component, where the message was originally created. Since this field is
	 * mainly for informational purposes, it is therefore optional and unspecified values shall
	 * be listed as an empty string, i.e., “”.
	 *
	 * @return The source of the message
	 */
    string_type const &get_source() const noexcept;

    /*!
     * This function returns a reference to facilitate copy-free usage or auto-update
     *
     * @return The source of the message
     */
    string_type &get_source() noexcept;

    /*!
     * @brief Set the source
     *
     * This function copies the given string
     *
     * @param[in] The source of the message
     */
    void set_source(string_type const &) noexcept;

	/*!
	 * The type of encoding to use for the payload, as text. This field is optional, and unspecified
	 * values shall be listed as an empty string, i.e., “”. However, the TMX system will automatically
	 * set this value upon encoding based on the type of data discovered inside the message container.
	 * If this field is found to be empty when decoding is attemped, then the default encoding is
	 * assumed based on the leading characters in the payload. For example, if a { is detected, then
	 * the payload is assumed to be JSON-encoded. As a last resort, the message payload is assumed to
	 * be a basic string whose contents are the supplied bytes.
	 *
	 * @return The payload encoding
	 */
    string_type const &get_encoding() const noexcept;

    /*!
     * This function returns a reference to facilitate copy-free usage or auto-update
     *
     * @return The payload encoding
     */
    string_type &get_encoding() noexcept;

    /*!
     * @brief Set the payload encoding
     *
     * This function copies the given string
     *
     * @param[in] The payload encoding
     */
    void set_encoding(string_type const &) noexcept;

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
	 *
	 * @return The metadata
	 */
	int_type get_metadata() const noexcept;

    /*!
     * @brief Set the metadata by numeric value.
     *
     * @param[in] The metadata
     */
    void set_metadata(int_type) noexcept;

    /*!
     * @brief Set the metadata via the given bit-set
     *
     * @param[in] The metadata
     */
    void set_metadata(std::bitset<common::types::Intmax::numBits>) noexcept;

private:
    // Some pre-defined etadata fields
    common::types::UInt<TMX_METADATA_QOS_BITS> _QoS;
    common::types::UInt<TMX_METADATA_PRIORITY_BITS> _priority;
    common::types::UInt<TMX_METADATA_BASE_BITS> _base;
    common::types::UInt<TMX_METADATA_ASSIGNMENT_GROUP_BITS> _assign_group;
    common::types::UInt<TMX_METADATA_ASSIGNMENT_ID_BITS> _assign_id;
    common::types::UInt<TMX_METADATA_FRAGMENT_BITS> _fragment;
    common::types::UInt<TMX_METADATA_ATTEMPT_BITS> _attempt;
    common::types::UInt<decltype(_QoS)::numBits + decltype(_priority)::numBits + decltype(_base)::numBits +
                        decltype(_assign_group)::numBits + decltype(_assign_id)::numBits +
                        decltype(_fragment)::numBits + decltype(_attempt)::numBits> _tmx_metadata;
    common::types::UInt<common::bit_size_of<typename decltype(_tmx_metadata)::value_type>() -
                        decltype(_tmx_metadata)::numBits> _reserved;
    common::types::UInt<common::bit_size_of<int_type>() - decltype(_tmx_metadata)::numBits -
                        decltype(_reserved)::numBits> _prog;

public:

    /*!
     * @return The quality of service specified for this message
     */
    typename decltype(_QoS)::value_type get_QoS() const noexcept;

    /*!
     * @brief Set the quality of service for this message
     *
     * @param[in] The quality of service as an integer
     */
    void set_QoS(typename decltype(_QoS)::value_type) noexcept;

    /*!
     * @return The priority specified for this message, not including the QoS
     */
    typename decltype(_priority)::value_type get_priority() const noexcept;

    /*!
     * @brief Set the priority for this message, not including the QoS
     *
     * @param[in] The priority
     */
    void set_priority(typename decltype(_priority)::value_type) noexcept;

    /*!
     * @return The base, i.e., 0 (auto), 16, 32, or 64) used for binary encoding of this message
     */
    typename decltype(_base)::value_type get_base() const noexcept;

    /*!
     * @brief Set the base, i.e., 0 (auto), 16, 32, or 64, used for binary encoding of this message
     *
     * @param[in] The base
     */
    void set_base(typename decltype(_base)::value_type) noexcept;

    /*!
     * @return The assignment group that this message belongs to, for handlers
     */
    typename decltype(_assign_group)::value_type get_assignment_group() const noexcept;

    /*!
     * @brief Set the assignment group that this message belongs to, for handlers
     *
     * @param[in] The group
     */
    void set_assignment_group(typename decltype(_assign_group)::value_type) noexcept;

    /*!
     * @return The unique identifier within the assignment group that this message belongs to, for handlers
     */
    typename decltype(_assign_id)::value_type get_assignment_id() const noexcept;

    /*!
     * @brief Set the unique identifier within the assignment group that this message belongs to, for handlers
     *
     * @param[in] The identifier
     */
    void set_assignment_id(typename decltype(_assign_id)::value_type) noexcept;

    /*!
     * @return The message fragment identifier, if the message is intentionally broken up by TMX
     */
    typename decltype(_fragment)::value_type get_fragment() const noexcept;

    /*!
     * @brief Set the message fragment identifier, if the message is intentionally broken up by TMX
     *
     * @oaram[in] The fragment
     */
    void set_fragment(typename decltype(_fragment)::value_type) noexcept;

    /*!
     * @return The message fragment identifier, if the message is intentionally broken up by TMX
     */
    typename decltype(_attempt)::value_type get_attempt() const noexcept;

    /*!
     * @brief Set the message fragment identifier, if the message is intentionally broken up by TMX
     *
     * @oaram[in] The fragment
     */
    void set_attempt(typename decltype(_attempt)::value_type) noexcept;

    /*!
     * @return Just the programmable portion of the metadata as an integer value
     */
    typename decltype(_prog)::value_type get_programmable_metadata() const noexcept;

    /*!
     * @brief Set the programmable portion of the metadata as an integer value
     *
     * @param[in] The metadata
     */
    void set_programmable_metadata(typename decltype(_prog)::value_type) noexcept;

    /*!
     * @brief Set the programmable portion of the metadata via the given bit-set
     *
     * @param[in] The metadata
     */
    void set_programmable_metadata(std::bitset<decltype(_prog)::numBits>) noexcept;

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
     *
	 * @return The timestamp as an integer value
	 */
    int_type get_timestamp() const noexcept;

    /*!
     * @brief Set the timestamp by numeric value.
     *
     * @param[in] The timestamp value
     */
    void set_timestamp(int_type) noexcept;

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
	inline auto get_timepoint() const noexcept {
		return std::chrono::time_point<Clock, Duration>(typename Clock::duration(this->get_timestamp()));
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
	inline auto set_timepoint(const std::chrono::time_point<Clock, Duration> &timepoint = Clock::now()) noexcept {
		this->set_timestamp(timepoint.time_since_epoch().count());
	}

	/*!
	 * @return The number of bytes in the message payload, to be added externally to the container only if needed
	 */
	typename string_type::size_type get_length() const noexcept;

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
	 *
     * @return The message payload as an (encoded) sequence of bytes
     */
    template <typename _CharT = byte_t>
    common::char_sequence<_CharT> get_payload() const noexcept {
        return common::to_char_sequence<_CharT>(this->get_payload_string().data());
    }

    /*!
     * This function returns a reference to facilitate copy-free usage or auto-update
     *
     * @return The message payload as an (encoded) sequence of characters
     */
    string_type &get_payload_string() noexcept;

    /*!
     * @return The message payload as an (encoded) sequence of characters
     */
    string_type const &get_payload_string() const noexcept;

    /*!
     * @brief Set the payload from the given string.
     *
     * This function copies the given string
     *
     * @param[in] The payload
     */
    void set_payload(string_type const &) noexcept;

    /*!
	 * @brief Set the payload from the given sequence of bytes.
	 *
    * @param[in] The byte sequence
	 */
    void set_payload(common::byte_sequence const &) noexcept;

    /*!
     * @brief Set the payload from the given string.
     *
     * This function copies the given string
     *
     * @param[in] The payload
     */
    template <typename _CharT>
    void set_payload(_CharT const *bytes, typename string_type::size_type len) {
        this->set_payload(common::to_byte_sequence(bytes, len));
    }

private:
    // Used for obtaining quick references
    string_type *_id = nullptr;
    string_type *_topic = nullptr;
    string_type *_source = nullptr;
    string_type *_encoding = nullptr;
    int_type *_metadata = nullptr;
    int_type *_timestamp = nullptr;
    string_type *_payload = nullptr;
};

} /* End namespace message */
} /* End namespace message */

#endif /* INCLUDE_TMX_MESSAGE_TMXMESSAGE_HPP_ */
