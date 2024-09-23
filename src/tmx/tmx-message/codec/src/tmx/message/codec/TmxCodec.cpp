/*!
 * Copyright (c) 2021 Battelle Memorial Institute
 *
 * All Rights Reserved.
 *
 * @file TmxByteEncoder.cpp
 *
 *  Created on: Aug 2, 2021
 *      @author: gmb
 */

#include <tmx/message/codec/TmxCodec.hpp>

#include <tmx/common/TmxTypeRegistry.hpp>

#include <memory>
#include <sstream>

using namespace tmx::common;

namespace tmx {
namespace message {
namespace codec {

static TmxTypeRegistry &get_encoder_registry() {
    static TmxTypeRegistry _reg { "tmx.message.codec.encoders" };
    return _reg;
}

static TmxTypeRegistry &get_decoder_registry() {
    static TmxTypeRegistry _reg { "tmx.message.codec.decoders" };
    return _reg;
}

void TmxEncoder::register_encoder() const {
    const auto &_descr = this->get_descriptor();
    get_encoder_registry().register_type(_descr.get_instance(), _descr.get_typeid(), _descr.get_type_name());
}

std::shared_ptr<const TmxEncoder> TmxEncoder::get_encoder(common::const_string name) {
    auto _descriptor = get_encoder_registry().get(name);
    if (_descriptor && _descriptor.get_instance())
        return _descriptor.as_instance<TmxEncoder>();

    return { };
}

void TmxDecoder::register_decoder() const {
    const auto &_descr = this->get_descriptor();
    get_decoder_registry().register_type(_descr.get_instance(), _descr.get_typeid(), _descr.get_type_name());
}

std::shared_ptr<const TmxDecoder> TmxDecoder::get_decoder(common::const_string name) {
    auto _descriptor = get_decoder_registry().get(name);
    if (_descriptor && _descriptor.get_instance())
        return _descriptor.as_instance<TmxDecoder>();

    return { };
}

TmxTypeDescriptor TmxEncoder::get_descriptor() const noexcept {
    std::shared_ptr<const void> _ptr { static_cast<const void *>(this), [](auto *) { } };
    return { _ptr, typeid(void), "" };
}

TmxTypeDescriptor TmxDecoder::get_descriptor() const noexcept {
    std::shared_ptr<const void> _ptr { static_cast<const void *>(this), [](auto *) { } };
    return { _ptr, typeid(void), "" };
}

bool TmxEncoder::is_binary() const noexcept {
    return true;
}

bool TmxDecoder::is_binary() const noexcept {
    return true;
}

TmxCodec::TmxCodec(TmxMessage const &message) noexcept: _message(message) { };

TmxMessage &TmxCodec::get_message() noexcept { return this->_message; };

std::basic_string<common::byte_t> TmxCodec::get_payload_bytes() const {
    // Determine if the encoder produced a binary output
    auto encoder = TmxEncoder::get_encoder(this->_message.get_encoding());
    if (encoder && encoder->is_binary()) {
        // Decode the bytes
        auto chars = this->_message.get_payload<common::char_t>();

        switch (this->_message.get_base()) {
            case 0:
                return byte_string_decode(chars, TMX_DEFAULT_BYTE_ENCODING);
            case 1:
                return byte_string_decode(chars, base16::value);
            case 2:
                return byte_string_decode(chars, base32::value);
            case 3:
                return byte_string_decode(chars, base64::value);
        }
    }

    // Use the payload string directly
    auto bytes = to_byte_sequence(this->_message.get_payload_string().c_str(), this->_message.get_length());
    return { bytes.data(), bytes.length() };
}

TmxError TmxCodec::encode(const common::types::Any &data, common::const_string codec) {
    if (codec == empty_string())
        codec = this->_message.get_encoding();

    std::shared_ptr<const TmxEncoder> encoder;
    if (codec == empty_string())
        encoder = TmxEncoder::get_encoder();
    else
        encoder = TmxEncoder::get_encoder(codec);

    if (!encoder) {
        std::string err { "TMX codec " };
        err.append(codec);
        err.append(" is not supported.");

        return { 11, err };
    }

    if (!this->_message.get_timestamp())
        this->_message.set_timepoint();
    if (this->_message.get_encoding().empty())
        this->_message.set_encoding(encoder->get_descriptor().get_type_short_name());
    if (this->_message.get_id().empty())
        this->_message.set_id(common::types::contents(data).get_type_short_name());

    std::ostringstream byte_stream;

    TmxError ret = encoder->encode(data, byte_stream);
    if (ret) return ret;

    std::string const &str = byte_stream.str();
    this->_message.set_payload(str.c_str(), str.length());
    return { };
}

TmxError TmxCodec::decode(common::types::Any &data, common::const_string schema) {
    if (schema == empty_string())
        schema = common::types::contents(data).get_type_name();

    std::shared_ptr<const TmxDecoder> decoder;
    if (this->_message.get_encoding() == empty_string())
        decoder = TmxDecoder::get_decoder();
    else
        decoder = TmxDecoder::get_decoder(this->_message.get_encoding());

    if (!decoder) {
        std::string err { "TMX codec " };
        err.append(this->_message.get_encoding());
        err.append(" is not supported.");

        return { 11, err };
    }

    auto const &b = this->_message.get_payload();
    std::istringstream is(to_char_sequence(b.data(), b.length()).data());
    return decoder->decode(data, TmxTypeRegistry().get(schema), is);
}

} /* End namespace codec */
} /* End namespace message */
} /* End namespace tmx */
