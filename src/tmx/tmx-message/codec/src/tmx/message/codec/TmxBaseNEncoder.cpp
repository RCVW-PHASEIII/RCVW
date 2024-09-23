/*!
 * Copyright (c) 2023 Battelle Memorial Institute
 *
 * All Rights Reserved.
 *
 * @file TmxBaseNEncoder.cpp
 *
 *  Created on: Jul 19, 2023
 *      @author: Gregory M. Baumgardner
 */

#include <tmx/common/TmxError.hpp>
#include <tmx/common/TmxTypeDescriptor.hpp>
#include <tmx/common/TmxTypeRegistrar.hpp>
#include <tmx/common/TmxTypeRegistry.hpp>
#include <tmx/message/codec/TmxCodec.hpp>

using namespace tmx::common;
using namespace tmx::common::types;

namespace tmx {
namespace message {
namespace codec {

template <std::uint8_t _N>
class TmxBaseNEncoder: public TmxEncoder {
    typedef TmxBaseNEncoder<_N> self_type;
    typedef common::_base_N<_N> traits_type;

public:
    TmxBaseNEncoder() {
        this->register_encoder();
    }

    common::TmxTypeDescriptor get_descriptor() const noexcept override {
        static std::string _name = "base-" + std::to_string(_N);
        static const auto &_descr = TmxEncoder::get_descriptor();
        return { _descr.get_instance(), typeid(self_type), _name.c_str() };
    }

    template <typename _Tp>
    TmxError handle(_Tp const &obj, byte_stream &os) const {
        os << byte_string_encode_value((TmxValueTypeOf<_Tp>)obj, traits_type::value);
        return { };
    }

    TmxError handle(Null const &obj, byte_stream &os) const {
        // Do nothing
        return { };
    }

    TmxError handle(String8 const &obj, byte_stream &os) const {
        os << byte_string_encode(to_byte_sequence(obj.c_str()), traits_type::value);
        return { };
    }

    template <typename _E>
    TmxError handle(Enum<_E> const &obj, byte_stream &os) const {
        return this->handle(obj.get_integer_value(), os);
    }

//    TmxError execute(TmxTypeDescriptor const &type, TmxArgList &args) const override {
//        TmxVariant<TmxScalarTypes, String8> var;
//
//        auto err = set_variant(var, type);
//        if (err) return err;
//
//        return do_handle(TmxTypeHandlerOverload {
//                [this](auto &v, TmxArgList &args) -> TmxError {
//                    std::shared_ptr<byte_stream> *ptr;
//                    if (args.size() > 1)
//                        ptr = tmx::common::any_cast< std::shared_ptr<byte_stream> >(&args[1]);
//
//                    if (ptr && ptr->get())
//                        return this->handle(make_type(v), *(ptr->get()));
//
//                    return { EINVAL, "Invalid argument: Missing output stream to encode to." };
//                }
//        }, var, args);
//   }
 };

template <std::uint8_t _N>
class TmxBaseNDecoder: public TmxDecoder {
    typedef TmxBaseNDecoder<_N> self_type;
    typedef common::_base_N<_N> traits_type;

public:
    TmxBaseNDecoder() {
        this->register_decoder();
    }

    common::TmxTypeDescriptor get_descriptor() const noexcept override {
        static std::string _name = "base-" + std::to_string(_N);
        static const auto &_descr = TmxDecoder::get_descriptor();
        return { _descr.get_instance(), typeid(self_type), _name.c_str() };
    }
};


static common::TmxTypeRegistrar< TmxBaseNEncoder<2> > _binary_encoder_registrar;
static common::TmxTypeRegistrar< TmxBaseNDecoder<2> > _binary_decoder_registrar;
static common::TmxTypeRegistrar< TmxBaseNEncoder<8> > _octal_encoder_registrar;
static common::TmxTypeRegistrar< TmxBaseNDecoder<8> > _octal_decoder_registrar;
static common::TmxTypeRegistrar< TmxBaseNEncoder<16> > _hexadecimal_encoder_registrar;
static common::TmxTypeRegistrar< TmxBaseNDecoder<16> > _hexadecimal_decoder_registrar;
static common::TmxTypeRegistrar< TmxBaseNEncoder<32> > _base32_encoder_registrar;
static common::TmxTypeRegistrar< TmxBaseNDecoder<32> > _base32_decoder_registrar;
static common::TmxTypeRegistrar< TmxBaseNEncoder<64> > _base64_encoder_registrar;
static common::TmxTypeRegistrar< TmxBaseNDecoder<64> > _base64_decoder_registrar;


} /* End namespace codec */
} /* End namespace message */
} /* End namespace tmx */