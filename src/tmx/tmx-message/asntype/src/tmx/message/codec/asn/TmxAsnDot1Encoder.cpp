/*!
 * Copyright (c) 2023 Battelle Memorial Institute
 *
 * All Rights Reserved.
 *
 * @file TmxAsnDot1Encoder.cpp
 *
 *  Created on: Aug 07, 2023
 *      @author: Gregory M. Baumgardner
 */
#include <tmx/message/codec/asn/TmxAsnDot1Schema.hpp>

#include <tmx/common/TmxError.hpp>
#include <tmx/common/TmxTypeDescriptor.hpp>
#include <tmx/common/TmxTypeRegistrar.hpp>
#include <tmx/common/TmxTypeRegistry.hpp>
#include <tmx/message/codec/TmxCodec.hpp>
#include <tmx/message/codec/asn/asn_application.h>
#include <tmx/message/codec/asn/BOOLEAN.h>
#include <tmx/message/codec/asn/INT8.h>
#include <tmx/message/codec/asn/INT16.h>
#include <tmx/message/codec/asn/INT32.h>
#include <tmx/message/codec/asn/INT64.h>
#include <tmx/message/codec/asn/UINT8.h>
#include <tmx/message/codec/asn/UINT16.h>
#include <tmx/message/codec/asn/UINT32.h>
#include <tmx/message/codec/asn/UINT64.h>
#include <tmx/message/codec/asn/FLOAT32.h>
#include <tmx/message/codec/asn/FLOAT64.h>
#include <tmx/message/codec/asn/FLOAT128.h>
#include <tmx/message/codec/asn/STRING8.h>
#include <tmx/message/codec/asn/ARRAY.h>
#include <tmx/message/codec/asn/PROPERTIES.h>

#include <functional>
#include <memory>
#include <ostream>

using namespace tmx::common;
using namespace tmx::common::types;

namespace tmx {
namespace message {
namespace codec {
namespace asn {
namespace schema {

asn_TYPE_descriptor_t const *get_schema(Null const &) {
    return nullptr;
}

asn_TYPE_descriptor_t const *get_schema(Boolean const &) {
    return &asn_DEF_BOOLEAN;
}

template <std::uint8_t _Sz>
asn_TYPE_descriptor_t const *get_schema(UInt < _Sz >
const &obj) {
if (_Sz < 16)
return &
asn_DEF_UINT8;
else if (_Sz < 32)
return &
asn_DEF_UINT16;
else if (_Sz < 64)
return &
asn_DEF_UINT32;
else
return &
asn_DEF_UINT64;
}

template <std::uint8_t _Sz>
asn_TYPE_descriptor_t const *get_schema(Int < _Sz >
const &obj) {
if (_Sz < 16)
return &
asn_DEF_INT8;
else if (_Sz < 32)
return &
asn_DEF_INT16;
else if (_Sz < 64)
return &
asn_DEF_INT32;
else
return &
asn_DEF_INT64;
}

template <std::uint8_t _Sz>
asn_TYPE_descriptor_t const *get_schema(Float < _Sz >
const &obj) {
if (_Sz < 64)
return &
asn_DEF_FLOAT32;
else if (_Sz < 128)
return &
asn_DEF_FLOAT64;
else
return &
asn_DEF_FLOAT128;
}

asn_TYPE_descriptor_t const *get_schema(String8 const &obj) {
    return &asn_DEF_STRING8;
}

asn_TYPE_descriptor_t const *get_schema(TmxTypeDescriptor const &descr) noexcept {
    TmxVariant<TmxNullTypes, TmxArithmeticTypes, String8> var;

    auto err = set_variant(var, descr);
    if (!err)
        return std::visit([](auto &v) -> asn_TYPE_descriptor_t const * { return get_schema(make_type(v)); }, var);

    auto ptr = descr.as_instance<Functor<asn_TYPE_descriptor_t const *> >();
    if (ptr && ptr->operator bool())
        return ptr->execute();

    return nullptr;
}

} /* End namespace schema */


template <typename _Name, asn_transfer_syntax _Syntax>
class TmxAsnDot1Encoder : public TmxEncoder {
    typedef TmxAsnDot1Encoder<_Name, _Syntax> self_type;

public:
    TmxAsnDot1Encoder() {
        this->register_encoder();
    }

    common::TmxTypeDescriptor get_descriptor() const noexcept override {
        static constexpr auto _name = concat(TMX_STATIC_STRING("asn.1-"), _Name{ });
        static const auto &_descr = TmxEncoder::get_descriptor();
        return { _descr.get_instance(), typeid(self_type), _name.c_str() };
    }

    TmxError handle(Null const &obj, byte_stream &os) const {
        // Do nothing
        return { };
    }

    TmxError handle(Boolean const &obj, byte_stream &os) const {
        BOOLEAN_t val = (obj ? 1 : 0);
        return do_encode(schema::get_schema(obj), &val, os);
    }

    template <std::uint8_t _Sz>
    TmxError handle(UInt <_Sz> const &obj, byte_stream &os) const {
        UINT64_t val;
        val.buf = nullptr;
        val.size = 0;
        asn_long2INTEGER(&val, obj);
        return do_encode(schema::get_schema(obj), &val, os);
    }

    template <std::uint8_t _Sz>
    TmxError handle(Int <_Sz> const &obj, byte_stream &os) const {
        INT64_t val;
        val.buf = nullptr;
        val.size = 0;
        asn_long2INTEGER(&val, obj);
        return do_encode(schema::get_schema(obj), &val, os);
    }

    template <std::uint8_t _Sz>
    TmxError handle(Float <_Sz> const &obj, byte_stream &os) const {
        FLOAT128_t val;
        val.buf = nullptr;
        val.size = 0;
        asn_double2REAL(&val, obj);
        return do_encode(schema::get_schema(obj), &val, os);
    }

    TmxError handle(String8 const &obj, byte_stream &os) const {
        STRING8_t val;
        val.buf = reinterpret_cast<std::uint8_t *>(const_cast<char *>(obj.data()));
        val.size = obj.length();
        return do_encode(schema::get_schema(obj), &val, os);
    }

    template <typename _E>
    TmxError handle(Enum <_E> const &obj, byte_stream &os) const {
        return this->handle(TmxTypeOf<typename Enum<_E>::underlying_type>(obj.get_integer_value()), os);
    }

    TmxError execute(TmxTypeDescriptor const &descr, std::reference_wrapper<TmxArgList> args) const override {
        std::shared_ptr<byte_stream> *ptr;

        if (args.get().size() > 1)
            ptr = tmx::common::any_cast<std::shared_ptr<byte_stream> >(&(args.get())[1]);

        if (!ptr || !ptr->get())
            return { EINVAL, "Invalid argument: Missing output stream to encode to." };

        auto &bytes = *(ptr->get());

        TmxVariant<TmxScalarTypes, String8> var;

        auto err = set_variant(var, descr);
        if (!err)
            return do_handle(TmxTypeHandlerOverload{
                    [this, &bytes](auto &v, TmxArgList &args) -> TmxError {
                        return this->handle(make_type(v), bytes);
                    }}, var, args);


        // See if this descriptor has an ASN.1 schema
        auto asn1 = schema::get_schema(descr);
        if (asn1) {
            // TODO
        }

        return { ENOTSUP, "Could not determine schema for " + descr.get_type_name() };
    };

    template <typename _Tp>
    TmxError do_encode(asn_TYPE_descriptor_t const *descriptor, _Tp *value, byte_stream &os) const {
        auto ret = asn_encode_to_new_buffer(nullptr, _Syntax, descriptor, value);
        if (ret.result.encoded < 0 || !ret.buffer) {
            std::string err{ "ASN.1 serialization of " };
            err.append(type_fqname<_Tp>());
            err.append(" type failed at tag ");
            err.append(ret.result.failed_type ? ret.result.failed_type->name : "????");
            err.append(".");
            return { ret.result.encoded, err };
        }

        // For XML encoding, use the actual string
        // Otherwise, convert to bytes
        static const_string nm{ _Name::c_str() };
        const_string str{ (typename byte_stream::char_type *) ret.buffer };
        if (str[str.length() - 1] == '\n')
            str = str.substr(0, str.length() - 1);

        if (nm.length() && nm[0] == 'x') {
            os << str;
        } else {
            os << byte_string_encode(to_byte_sequence(str.data(), str.length()));
        }

        free(ret.buffer);
        return { };
    }
};

static common::TmxTypeRegistrar< TmxAsnDot1Encoder<decltype(TMX_STATIC_STRING("ber")), ATS_DER> > _asn1ber_encoder_registrar;
static common::TmxTypeRegistrar< TmxAsnDot1Encoder<decltype(TMX_STATIC_STRING("xer")), ATS_BASIC_XER> > _asn1xer_encoder_registrar;
static common::TmxTypeRegistrar< TmxAsnDot1Encoder<decltype(TMX_STATIC_STRING("oer")), ATS_BASIC_OER> > _asn1oer_encoder_registrar;
static common::TmxTypeRegistrar< TmxAsnDot1Encoder<decltype(TMX_STATIC_STRING("uper")), ATS_UNALIGNED_BASIC_PER> > _asn1uper_encoder_registrar;

namespace schema {

common::TmxError dump(TmxTypeDescriptor const &descr, const void *obj, Any &out) noexcept {
    std::stringstream ss;

    auto descriptor = get_schema(descr);
    if (descriptor) {
        // Always serialize to XML
        _asn1xer_encoder_registrar.instance()->do_encode(descriptor, obj, ss);
        auto decoder = message::codec::TmxDecoder::get_decoder("xml");
        if (decoder) {
            // It is byte encoded
            auto bytes = byte_string_decode(ss.str().c_str());
            return decoder->decode(out, to_char_sequence(bytes.data()));
        }
    }

    return { ENOTSUP, "Could not determine schema for " + descr.get_type_name() };
}

} /* End namespace schema */

template <typename _Name, asn_transfer_syntax _Syntax>
class TmxAsnDot1Decoder: public TmxDecoder {
    typedef TmxAsnDot1Decoder<_Name, _Syntax> self_type;
    const TmxTypeRegistry _registry { "tmx.message.codec.schemas.asn1" };

public:
    TmxAsnDot1Decoder() {
        this->register_decoder();
    }

    common::TmxTypeDescriptor get_descriptor() const noexcept override {
        static constexpr auto _name = concat(TMX_STATIC_STRING("asn.1-"), _Name { });
        static const auto &_descr = TmxDecoder::get_descriptor();
        return { _descr.get_instance(), typeid(self_type), _name.c_str() };
    }

    TmxError handle(Null const &obj, byte_sequence bytes, Any &data) const {
        // Do nothing
        data.emplace<Null>();
        return { };
    }

    TmxError handle(Boolean const &obj, byte_sequence bytes, Any &data) const {
        BOOLEAN_t val;
        auto *p = &val;
        auto ret = do_decode(schema::get_schema(obj), &p, bytes);
        if (ret) return ret;

        data.emplace<Boolean>(val != 0);
        return { };
    }

    template <std::uint8_t _Sz>
    TmxError handle(UInt<_Sz> const &obj, byte_sequence bytes, Any &data) const {
        UINT64_t val;
        auto *p = &val;
        auto ret = do_decode(schema::get_schema(obj), &p, bytes);
        if (ret) return ret;

        uintmax_t v;
        asn_INTEGER2umax(&val, &v);
        data.emplace< UInt<_Sz> >((TmxValueTypeOf< UInt<_Sz> >)v);
        return { };
    }

    template <std::uint8_t _Sz>
    TmxError handle(Int<_Sz> const &obj, byte_sequence bytes, Any &data) const {
        INT64_t val;
        auto *p = &val;
        auto ret = do_decode(schema::get_schema(obj), &p, bytes);
        if (ret) return ret;

        intmax_t v;
        asn_INTEGER2imax(&val, &v);
        data.emplace< Int<_Sz> >((TmxValueTypeOf< Int<_Sz> >)v);
        return { };
    }

    template <std::uint8_t _Sz>
    TmxError handle(Float<_Sz> const &obj, byte_sequence bytes, Any &data) const {
        FLOAT128_t val;
        auto *p = &val;
        auto ret = do_decode(schema::get_schema(obj), &p, bytes);
        if (ret) return ret;

        double v;
        asn_REAL2double(&val, &v);
        data.emplace< Float<_Sz> >((TmxValueTypeOf< Float<_Sz> >)v);
        return { };
    }

    TmxError handle(String8 const &obj, byte_sequence bytes, Any &data) const {
        STRING8_t val;
        auto *p = &val;
        auto ret = do_decode(schema::get_schema(obj), &p, bytes);
        if (ret) return ret;

        data.emplace<String8>(to_char_sequence<typename String8::char_t>(val.buf, val.size));
        return { };
    }

    template <typename _E>
    TmxError handle(Enum<_E> const &obj, byte_sequence bytes, Any &data) const {
        String8 _tmp;
        auto ret = this->handle(_tmp, bytes, data);
        if (ret) return ret;

        _tmp = tmx::common::any_cast<String8>(data);
        data.emplace< Enum<_E> >(-1, _tmp.c_str());
        return { };
    }

    TmxError execute(TmxTypeDescriptor const &descr, std::reference_wrapper<TmxArgList> args) const override {
        byte_sequence *bytes = nullptr;
        if (args.get().size() > 0)
            bytes = tmx::common::any_cast<byte_sequence>(&(args.get())[0]);

        if (!bytes)
            return { EINVAL, "Invalid argument: No byte sequence to decode from." };

        TmxVariant<TmxScalarTypes, String8> var;

        auto err = set_variant(var, descr);
        if (!err)
            return do_handle(TmxTypeHandlerOverload{
                [this, &bytes](auto &v, TmxArgList &args) -> TmxError {
                    // Bytes is a string
                    byte_sequence b { byte_string_decode(to_char_sequence(*bytes)).c_str() };
                    TLOG(INFO) << to_char_sequence(*bytes);
                    TLOG(INFO) << b;
                    return this->handle(make_type(v), b, args.emplace_back());
                }}, var, args);

        // See if this descriptor has an ASN.1 schema
        auto asn1 = schema::get_schema(descr);
        if (asn1) {
            // Make sure this
        }

        return { ENOTSUP, "Could not determine schema for " + descr.get_type_name() };
    }

    template <typename _Tp>
    TmxError do_decode(asn_TYPE_descriptor_t const *descriptor, _Tp **value, byte_sequence &bytes) const {
        auto ret = asn_decode(nullptr, _Syntax, descriptor, (void **)value, bytes.data(), bytes.length());
        if (ret.code == RC_OK)
            return { };

        std::string err { "ASN.1 decoding of " };
        err.append(type_fqname<_Tp>());
        err.append(" type from ");
        err.append(byte_string_encode(bytes));
        err.append(" failed after ");
        err.append(std::to_string(ret.consumed));
        err.append(" bytes.");
        return { ret.code, err };
    }
};

static common::TmxTypeRegistrar< TmxAsnDot1Decoder<decltype(TMX_STATIC_STRING("ber")), ATS_DER> > _asn1ber_decoder_registrar;
static common::TmxTypeRegistrar< TmxAsnDot1Decoder<decltype(TMX_STATIC_STRING("xer")), ATS_BASIC_XER> > _asn1xer_decoder_registrar;
static common::TmxTypeRegistrar< TmxAsnDot1Decoder<decltype(TMX_STATIC_STRING("oer")), ATS_BASIC_OER> > _asn1oer_decoder_registrar;
static common::TmxTypeRegistrar< TmxAsnDot1Decoder<decltype(TMX_STATIC_STRING("uper")), ATS_UNALIGNED_BASIC_PER> > _asn1uper_decoder_registrar;

namespace schema {

common::TmxError load(TmxTypeDescriptor const &descr, Any const &obj, void **out) noexcept {
    std::stringstream ss;
    auto descriptor = get_schema(descr);
    if (descriptor) {
        // Always serialize from XML
        auto encoder = codec::TmxEncoder::get_encoder("xml");
        if (encoder) {
            auto ret = encoder->encode(obj, ss);
            if (ret) return ret;
        }

        auto bytes = to_byte_sequence(ss.str().c_str());
        return _asn1xer_decoder_registrar.instance()->do_decode(descriptor, out, bytes);
    }

    return { ENOTSUP, "Could not determine schema for " + descr.get_type_name() };
}

} /* End namespace schema */
} /* End namespace asn */
} /* End namespace codec */
} /* End namespace message */
} /* End namespace tmx */