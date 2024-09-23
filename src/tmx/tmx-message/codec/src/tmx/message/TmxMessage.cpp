/*!
 * Copyright (c) 2021 Battelle Memorial Institute
 *
 * All Rights Reserved.
 *
 * @file TmxMessage.cpp
 *
 *  Created on: Jun 16, 2021
 *      @author: Gregory M. Baumgardner
 */

#include <tmx/message/TmxMessage.hpp>

#include <tmx/common/TmxTypeRegistrar.hpp>
#include <tmx/message/TmxData.hpp>

#include <memory>

using namespace std;
using namespace tmx::common;
using namespace tmx::common::types;

namespace tmx {
namespace message {

enum class TmxMessageField: std::int8_t {
    id,
    topic,
    source,
    encoding,
    metadata,
    timestamp,
    payload
};

TmxMessage::TmxMessage() noexcept: TmxData() {
    TmxData _data { this->get_container() };
    this->_payload = &(_data[TmxMessageField::payload].get_container().emplace<string_type>());
    this->_encoding = &(_data[TmxMessageField::encoding].get_container().emplace<string_type>());
    this->_metadata = &(_data[TmxMessageField::metadata].get_container().emplace<int_type>(0));
    this->_timestamp = &(_data[TmxMessageField::timestamp].get_container().emplace<int_type>(0));
    this->_source = &(_data[TmxMessageField::source].get_container().emplace<string_type>());
    this->_topic = &(_data[TmxMessageField::topic].get_container().emplace<string_type>());
    this->_id = &(_data[TmxMessageField::id].get_container().emplace<string_type>());

    // Break up the metadata
    this->set_metadata(*(this->_metadata));
}

TmxMessage::TmxMessage(const TmxMessage &other) noexcept: TmxMessage() {
    this->operator=(other);
}
TmxMessage::TmxMessage(TmxData &&data) noexcept: TmxMessage() {
    this->operator=(data);
}

TmxMessage& TmxMessage::operator=(TmxMessage const &other) noexcept {
    return this->operator=(other.get_container());
}

TmxMessage& TmxMessage::operator=(TmxData const &other) noexcept {
    if (other[TmxMessageField::payload]) this->set_payload(other[TmxMessageField::payload]);
    if (other[TmxMessageField::metadata]) this->set_metadata(other[TmxMessageField::metadata]);
    if (other[TmxMessageField::timestamp]) this->set_timestamp(other[TmxMessageField::timestamp]);
    if (other[TmxMessageField::encoding]) this->set_encoding(other[TmxMessageField::encoding]);
    if (other[TmxMessageField::source]) this->set_source(other[TmxMessageField::source]);
    if (other[TmxMessageField::topic]) this->set_topic(other[TmxMessageField::topic]);
    if (other[TmxMessageField::id]) this->set_id(other[TmxMessageField::id]);

    return *this;
}

typename TmxMessage::string_type &TmxMessage::get_id() noexcept {
    return *(this->_id);
}

typename TmxMessage::string_type const &TmxMessage::get_id() const noexcept {
    return *(this->_id);
}

typename TmxMessage::string_type const &TmxMessage::get_topic() const noexcept {
    return *(this->_topic);
}

typename TmxMessage::string_type &TmxMessage::get_topic() noexcept {
    return *(this->_topic);
}

typename TmxMessage::string_type const &TmxMessage::get_source() const noexcept {
    return *(this->_source);
}

typename TmxMessage::string_type &TmxMessage::get_source() noexcept {
    return *(this->_source);
}

typename TmxMessage::string_type const &TmxMessage::get_encoding() const noexcept {
    return *(this->_encoding);
}

typename TmxMessage::string_type &TmxMessage::get_encoding() noexcept {
    return *(this->_encoding);
}

typename TmxMessage::string_type const &TmxMessage::get_payload_string() const noexcept {
    return *(this->_payload);
}

typename TmxMessage::string_type &TmxMessage::get_payload_string() noexcept {
    return *(this->_payload);
}

typename TmxMessage::int_type TmxMessage::get_metadata() const noexcept {
    return *(this->_metadata);
}

typename TmxMessage::int_type TmxMessage::get_timestamp() const noexcept {
    return *(this->_timestamp);
}

typename decltype(TmxMessage::_QoS)::value_type TmxMessage::get_QoS() const noexcept {
    return this->_QoS;
}

typename decltype(TmxMessage::_priority)::value_type TmxMessage::get_priority() const noexcept {
    return this->_priority;
}

typename decltype(TmxMessage::_base)::value_type TmxMessage::get_base() const noexcept {
    typename decltype(TmxMessage::_base)::value_type ret = this->_base;
    return ret ? 1 << (3 + ret) : 0;
}

typename decltype(TmxMessage::_assign_group)::value_type TmxMessage::get_assignment_group() const noexcept {
    return this->_assign_group;
}

typename decltype(TmxMessage::_assign_id)::value_type TmxMessage::get_assignment_id() const noexcept {
    return this->_assign_id;
}

typename decltype(TmxMessage::_fragment)::value_type TmxMessage::get_fragment() const noexcept {
    return this->_fragment;
}

typename decltype(TmxMessage::_attempt)::value_type TmxMessage::get_attempt() const noexcept {
    return this->_attempt;
}

typename decltype(TmxMessage::_prog)::value_type TmxMessage::get_programmable_metadata() const noexcept {
    return this->_prog;
}

typename TmxMessage::string_type::size_type TmxMessage::get_length() const noexcept {
    return this->get_payload_string().length();
}

void TmxMessage::set_id(string_type const &value) noexcept {
    this->get_id().assign(value);
}

void TmxMessage::set_topic(string_type const &value) noexcept {
    this->get_topic().assign(value);
}

void TmxMessage::set_source(string_type const &value) noexcept {
    this->get_source().assign(value);
}

void TmxMessage::set_encoding(string_type const &value) noexcept {
    this->get_encoding().assign(value);
}

void TmxMessage::set_metadata(typename common::types::Intmax::value_type value) noexcept {
    *(this->_metadata) = value;

    // Reset the metadata fields
    types::unpack(value, _QoS, _priority, _base, _assign_group, _assign_id, _fragment, _attempt, _reserved, _prog);
}

void TmxMessage::set_metadata(std::bitset<common::types::Intmax::numBits> bits) noexcept {
    common::types::Intmax _tmp = bits.to_ullong();
    this->set_metadata(_tmp);
}

void TmxMessage::set_QoS(typename decltype(TmxMessage::_QoS)::value_type qos) noexcept {
    this->_QoS = qos;
    *(this->_metadata) = types::pack(_QoS, _priority, _base, _assign_group, _assign_id,
                                     _fragment, _attempt, _reserved, _prog);
}

void TmxMessage::set_priority(typename decltype(TmxMessage::_priority)::value_type p) noexcept {
    this->_priority = p;
    *(this->_metadata) = types::pack(_QoS, _priority, _base, _assign_group, _assign_id,
                                     _fragment, _attempt, _reserved, _prog);
}

void TmxMessage::set_base(typename decltype(TmxMessage::_base)::value_type base) noexcept {
    this->_base = (base >> 4);
    if (this->_base > decltype(TmxMessage::_base)::mask)
        this->_base = decltype(TmxMessage::_base)::mask;
    *(this->_metadata) = types::pack(_QoS, _priority, _base, _assign_group, _assign_id,
                                     _fragment, _attempt, _reserved, _prog);
}

void TmxMessage::set_assignment_group(typename decltype(TmxMessage::_assign_group)::value_type grp) noexcept {
    this->_assign_group = grp;
    *(this->_metadata) = types::pack(_QoS, _priority, _base, _assign_group, _assign_id,
                                     _fragment, _attempt, _reserved, _prog);
}

void TmxMessage::set_assignment_id(typename decltype(TmxMessage::_assign_id)::value_type id) noexcept {
    this->_assign_id = id;
    *(this->_metadata) = types::pack(_QoS, _priority, _base, _assign_group, _assign_id,
                                     _fragment, _attempt, _reserved, _prog);
}

void TmxMessage::set_fragment(typename decltype(TmxMessage::_fragment)::value_type f) noexcept {
    this->_fragment = f;
    *(this->_metadata) = types::pack(_QoS, _priority, _base, _assign_group, _assign_id,
                                     _fragment, _attempt, _reserved, _prog);
}

void TmxMessage::set_attempt(typename decltype(TmxMessage::_attempt)::value_type a) noexcept {
    this->_attempt = a;
    *(this->_metadata) = types::pack(_QoS, _priority, _base, _assign_group, _assign_id,
                                     _fragment, _attempt, _reserved, _prog);
}

void TmxMessage::set_programmable_metadata(typename decltype(TmxMessage::_prog)::value_type prog) noexcept {
    this->_prog = prog;
    *(this->_metadata) = types::pack(_QoS, _priority, _base, _assign_group, _assign_id,
                                     _fragment, _attempt, _reserved, _prog);
}

void TmxMessage::set_programmable_metadata(std::bitset<decltype(_prog)::numBits> bits) noexcept {
    common::types::UInt<decltype(_prog)::numBits> _tmp = bits.to_ullong();
    this->set_programmable_metadata(_tmp);
}

void TmxMessage::set_timestamp(typename common::types::Intmax::value_type value) noexcept {
    *(this->_timestamp) = value;
}

void TmxMessage::set_payload(typename TmxMessage::string_type const &value) noexcept {
    this->get_payload_string().assign(value);
}

void TmxMessage::set_payload(const common::byte_sequence &value) noexcept {
    const auto &chars = to_char_sequence(value);
    this->set_payload(typename TmxMessage::string_type(chars.data(), value.length()));
}

} /* End namespace message */
} /* End namespace message */


