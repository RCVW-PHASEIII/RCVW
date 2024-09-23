/*!
 * Copyright (c) 2021 Battelle Memorial Institute
 *
 * All Rights Reserved.
 *
 * @file TmxCodec.hpp
 *
 *  Created on: Aug 2, 2021
 *      @author: gmb
 */

#ifndef CODEC_INCLUDE_TMX_MESSAGE_CODEC_TMXCODEC_HPP_
#define CODEC_INCLUDE_TMX_MESSAGE_CODEC_TMXCODEC_HPP_

#include <tmx/platform.hpp>

#include <tmx/common/TmxError.hpp>
#include <tmx/common/TmxFunctor.hpp>
#include <tmx/common/TmxTypeHandler.hpp>
#include <tmx/common/types/Any.hpp>
#include <tmx/common/types/Array.hpp>
#include <tmx/message/TmxData.hpp>
#include <tmx/message/TmxMessage.hpp>

#include <iostream>
#include <istream>
#include <ostream>
#include <sstream>

#ifndef TMX_DEFAULT_CODEC
#define TMX_DEFAULT_CODEC "json"
#endif

namespace tmx {
namespace message {
namespace codec {

/*!
 * @brief An interface for encoding Any type to byte form
 */
class TmxEncoder: public common::TmxTypeHandler {
public:
    typedef std::basic_ostream<common::char_t> byte_stream;

    /*!
     * @brief Default destructor
     */
    virtual ~TmxEncoder() = default;

    /*!
     * @return A descriptor for this encoder
     */
    virtual common::TmxTypeDescriptor get_descriptor() const noexcept;

    /*!
     * @return True if this is a raw binary encoder, false if this is a character encoder
     */
    virtual bool is_binary() const noexcept;

    /*!
     * @return The TMX encoder registered for the given name
     */
    static std::shared_ptr<const TmxEncoder> get_encoder(common::const_string = TMX_DEFAULT_CODEC);

    /*!
     * @brief Encode the specified data to the output stream
     *
     * @param[in] data The data to encode
     * @param[in] os The output stream
     * @return Any error that occurs
     */
    template <typename _CharT = tmx::common::char_t>
    inline common::TmxError encode(common::types::Any const &data,
                                   std::basic_ostream<_CharT> &os = std::cout) const noexcept {
        std::basic_ostringstream<_CharT> ss;
        std::shared_ptr< byte_stream > _ptr { &ss, [](auto *) { }};
        common::TmxArgList _args { data, _ptr };

        auto _descr = common::TmxTypeRegistry().get(data.type(), true);
        common::TmxError result = this->execute(_descr, std::ref(_args));
        if (!result)
            os << ss.str();

        return result;
    }

protected:
    /*!
     * @brief Default constructor is protected so this interface cannot be used directly
     */
    TmxEncoder() = default;

    /*!
     * @brief Register this encoder
     */
    void register_encoder() const;
};

/*!
 * @brief An interface for decoding Any type from byte form
 */
class TmxDecoder: public common::TmxTypeHandler {
public:
    /*!
     * @brief Default destructor
     */
    virtual ~TmxDecoder() = default;

    /*!
     * @return A type descriptor for this decoder
     */
    virtual common::TmxTypeDescriptor get_descriptor() const noexcept;

    /*!
     * @return True if this is a raw binary decoder, false if this is a character decoder
     */
    virtual bool is_binary() const noexcept;

    /*!
     * @return The TMX decoder registered for the given name
     */
    static std::shared_ptr<const TmxDecoder> get_decoder(common::const_string = TMX_DEFAULT_CODEC);

    template <typename _CharT>
    inline common::TmxError decode(common::types::Any &data, common::TmxTypeDescriptor const &type,
                                   common::char_sequence<_CharT> const &str) const noexcept {
        common::TmxArgList _args { common::to_byte_sequence(str.data(), str.length()) };
        auto ret = this->execute(type, std::ref(_args));
        if (ret) return ret;

        if (_args.size() >= 2) {
            data = std::move(_args.back());
            return { };
        }

        return { 1, "Decoder failed to decode for unknown reason" };
    }

    /*!
     * @brief Decode the bytes from the input stream to the given data structure
     *
     * @param[out] data The data structure
     * @param[in] type The type to decode to
     * @param[in] is The input stream
     * @return Any error that occurs
     */
    template <typename _CharT = tmx::common::char_t>
    inline common::TmxError decode(common::types::Any &data, common::TmxTypeDescriptor const &type,
                       std::basic_istream<_CharT> &is = std::cin) const noexcept {

        std::basic_string<_CharT> str(std::istreambuf_iterator<_CharT>(is), { });
        return this->decode(data, type, common::to_char_sequence(str.c_str(), str.length()));
    }

    template <typename _Tp, typename _CharT = tmx::common::char_t>
    common::TmxError decode(_Tp &data, common::char_sequence<_CharT> const &str) const noexcept {
        std::shared_ptr<const void> _ptr { static_cast<const void *>(&data), [](auto *) { }};
        const common::TmxTypeDescriptor _descr { _ptr, typeid(_Tp), common::type_fqname<_Tp>().data() };

        common::types::Any _tmp;
        auto ret = this->decode(_tmp, _descr, str);
        if (!ret)
            // Try to extract the value
            ret = this->_extract(_tmp, data);

        return ret;
    }

    template <typename _Tp, typename _CharT = tmx::common::char_t>
    common::TmxError decode(_Tp &data, std::basic_istream<_CharT> &is = std::cin) const noexcept {
        std::basic_string<_CharT> str(std::istreambuf_iterator<_CharT>(is), { });
        return this->decode(data, common::to_char_sequence(str.c_str(), str.length()));
    }

protected:
    /*!
     * @brief Default constructor is protected so this interface cannot be used directly
     */
    TmxDecoder() = default;

    /*!
     * @brief Register this decoder
     */
    void register_decoder() const;

    template <typename _Tp>
    typename std::enable_if<std::is_copy_assignable<_Tp>::value, common::TmxError>::type
    _extractFrom(std::shared_ptr<_Tp> val, _Tp &data) const noexcept {
        if (!val)
            return { ENODATA,
                     "Data value does not contain a " + std::string(common::type_fqname(data).data()) + " type." };

        data = *val;
        return { };
    }

    template <typename _Tp>
    typename std::enable_if<!std::is_copy_assignable<_Tp>::value, common::TmxError>::type
    _extractFrom(std::shared_ptr<_Tp> val, _Tp &data) const noexcept {
        return { ENOSYS,
                 "It is not possible to copy assign a " + std::string(common::type_fqname(data).data()) + " type." };
    }

    template <typename _Tp>
    typename std::enable_if<std::is_assignable<_Tp, common::types::Any>::value, common::TmxError>::type
    _extract(common::types::Any &val, _Tp &data) const noexcept {
        if (this->_extractFrom(common::types::as<_Tp>(val), data))
            data = val;
        return { };
    }

    template <typename _Tp>
    typename std::enable_if<!std::is_assignable<_Tp, common::types::Any>::value, common::TmxError>::type
    _extract(common::types::Any &val, _Tp &data) const noexcept {
        return this->_extractFrom(common::types::as<_Tp>(val), data);
    }
};

/*!
 * @brief A helper class used to perform encoding and decoding operations on TMX messages
 *
 * Note that a new instance of this class should be used for each message
 * instance, which is enforced somewhat by disallowing copying and assigning.
 * To avoid extra copying, this class uses a
 */
class TmxCodec {
public:
    /*!
     * @brief Constructor
     */
    TmxCodec(TmxMessage const & = {}) noexcept;

    /*!
     * @brief Default destructor
     */
    ~TmxCodec() = default;

    /*!
     * @brief No copy constructor
     */
    TmxCodec(TmxCodec const &) = delete;

    /*!
     * @brief No assignment operator
     */
    TmxCodec &operator=(TmxCodec const &) = delete;

    /*!
     * @return A copy of the encoded TMX message
     */
    TmxMessage &get_message() noexcept;

    /*!
     * @brief Encode the data into the message
     *
     * This sets the basic contents of the message.
     *
     * If the encoder is specified as an argument, then that one
     * is used. Otherwise, the encoder is taken from the message.
     * If no specific encoder can be determined, then the default one
     * is used, which is typically JSON.
     *
     * Errors that occur during the encoding process will
     * likely result in an unusable message.
     *
     * @param[in] The data to encode to bytes
     * @param[in] The name of the encoder to use
     * @return An errors that occur during the encoding
     */
    common::TmxError encode(common::types::Any const &, common::const_string = common::empty_string());

    /*!
     * @brief Decode the message into a new type
     *
     * The message should contain the precise decoder to use, but
     * if none is specified, then the default one is used, which
     * is typically JSON.
     *
     * The second parameter is a hint for the decoder on the
     * possible structure of the encoded data. This most commonly
     * is the name of the class or schema to use for decoding.
     *
     * Errors that occur during the decoding process will
     * likely result in unusable data.
     *
     * @param[in] The data to decode the bytes to
     * @param[in] A hint on the possible structure of the data
     * @return An errors that occur during the encoding
     */
    common::TmxError decode(common::types::Any &, common::const_string = common::empty_string());

    /*!
     * @return The message payload as a new string of bytes
     */
    std::basic_string<common::byte_t> get_payload_bytes() const;

    /*!
     * @return The message payload as a new string
     */
    template <typename _CharT = common::char_t>
    std::basic_string<_CharT> get_payload_string() const {
        auto seq = common::to_char_sequence<_CharT>(this->_message.get_payload_string().c_str(),
                                                    this->_message.get_length());
        return { seq.data(), seq.length() };
    }
private:
    TmxMessage _message;
};

} /* End namespace codec */
} /* End namespace message */
} /* End namespace tmx */

#endif /* CODEC_INCLUDE_TMX_MESSAGE_CODEC_TMXCODEC_HPP_ */
