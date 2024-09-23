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

#include <tmx/common/TmxException.hpp>

#include <memory>

using namespace std;
using namespace tmx::common;
using namespace tmx::common::types;

namespace tmx {
namespace message {

TmxMessage::TmxMessage() { }

TmxMessage::~TmxMessage() { }

TmxMessage::TmxMessage(const TmxMessage &other):
		_type_id(other._type_id),
		_topic(other._topic),
		_source(other._source),
		_encoding(other._encoding),
		_timestamp(other._timestamp),
		_metadata(other._metadata),
		_payload(other._payload) {
}

TmxMessage::TmxMessage(TmxMessage &&other):
		_type_id(other._type_id),
		_topic(other._topic),
		_source(other._source),
		_encoding(other._encoding),
		_timestamp(other._timestamp),
		_metadata(other._metadata),
		_payload(other._payload) {
	other._payload.clear();
}

TmxMessage& TmxMessage::operator=(const TmxMessage &other) {
	this->_type_id = other._type_id;
	this->_topic = other._topic;
	this->_source = other._source;
	this->_encoding = other._encoding;
	this->_timestamp = other._timestamp;
	this->_metadata = other._metadata;
	this->_payload = other._payload;

	return *this;
}

TmxMessage& TmxMessage::operator=(TmxMessage &&other) {
	this->_type_id = other._type_id;
	this->_topic = other._topic;
	this->_source = other._source;
	this->_encoding = other._encoding;
	this->_timestamp = other._timestamp;
	this->_metadata = other._metadata;
	this->_payload = other._payload;
	other._payload.clear();

	return *this;
}

typename TmxMessage::bytes_t::size_type TmxMessage::get_length() const {
	return this->_payload.length();
}

TmxMessage::bytes_t TmxMessage::get_payload() const {
	return this->_payload;
}

void TmxMessage::set_payload(bytes_t bytes) {
	this->_payload = bytes.to_string();
}

} /* End namespace message */
} /* End namespace message */


