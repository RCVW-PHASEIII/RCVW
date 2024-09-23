/*!
 * Copyright (c) 2023 Battelle Memorial Institute
 *
 * All Rights Reserved.
 *
 * @file TmxDataSerializer.hpp
 *
 *  Created on: Aug 1, 2023
 *      @author: Gregory M. Baumgardner
 */

#ifndef CODEC_INCLUDE_TMX_MESSAGE_SERIALIZER_TMXDATASERIALIZER_HPP_
#define CODEC_INCLUDE_TMX_MESSAGE_SERIALIZER_TMXDATASERIALIZER_HPP_

#include <tmx/platform.hpp>

#include <tmx/common/TmxLogger.hpp>
#include <tmx/common/types/Any.hpp>
#include <tmx/message/TmxData.hpp>
#include <tmx/message/codec/TmxCodec.hpp>

#include <istream>
#include <ostream>
#include <type_traits>

namespace tmx {
namespace message {
namespace codec {
namespace serializer {

class encoding {
public:
    inline encoding(TmxData &&data, common::const_string const &codec = TMX_DEFAULT_CODEC) noexcept:
            _data(data), _codec(codec) { }

    template <typename _CharT>
    std::basic_ostream<_CharT> &operator<<(std::basic_ostream<_CharT> &os) const noexcept {
        auto encoder = codec::TmxEncoder::get_encoder(this->_codec);
        if (encoder) {
            encoder->encode(this->_data.get_container(), os);
        } else {
            TLOG(INFO) << "Attempt to serialize data without encoder " << this->_codec;
            os << "????????";
        }

        return os;
    }

    template <typename _CharT>
    std::basic_istream<_CharT> &operator>>(std::basic_istream<_CharT> &is) noexcept {
        auto decoder = codec::TmxDecoder::get_decoder(this->_codec);
        if (decoder) {
            decoder->decode(this->_data.get_container(), is);
        } else {
            TLOG(INFO) << "Attempt to deserialze data without decoder " << this->_codec;
            this->_data.get_container().emplace<common::types::Null>();
        }

        return is;
    }

private:
    TmxData _data;
    std::string const _codec;
};

template <typename _CharT>
inline std::basic_ostream<_CharT> &operator<<(std::basic_ostream<_CharT> &os, common::types::Any const &data) noexcept {
    return encoding(TmxData(data)).operator<<(os);
}

template <typename _CharT>
inline std::basic_ostream<_CharT> &operator<<(std::basic_ostream<_CharT> &os, TmxData const &data) noexcept {
    return operator<<(os, data.get_container());
}

template <typename _CharT>
inline std::basic_istream<_CharT> &operator>>(std::basic_istream<_CharT> &is, common::types::Any &data) noexcept {
    return encoding(TmxData(data)).operator>>(is);
}

template <typename _CharT>
inline std::basic_istream<_CharT> &operator>>(std::basic_istream<_CharT> &is, TmxData &data) {
    return operator>>(is, data.get_container());
}

}
}
}
}

#endif /* CODEC_INCLUDE_TMX_MESSAGE_SERIALIZER_TMXDATASERIALIZER_HPP_ */
