/*!
 * Copyright (c) 2023 Battelle Memorial Institute
 *
 * All Rights Reserved.
 *
 * @file TmxStringEncoder.cpp
 *
 *  Created on: Aug 04, 2023
 *      @author: Gregory M. Baumgardner
 */

#include <tmx/platform.hpp>

#include <tmx/common/TmxLogger.hpp>
#include <tmx/message/codec/TmxCodec.hpp>

#include <boost/algorithm/string.hpp>
#include <limits>
#include <regex>
#include <string>

using namespace tmx::common;
using namespace tmx::common::types;

namespace tmx {
namespace message {
namespace codec {

class TmxStringEncoder: public TmxEncoder {
public:
	TmxStringEncoder() {
        this->register_encoder();
    }

	TmxTypeDescriptor get_descriptor() const noexcept override {
		static const auto &_descr = TmxEncoder::get_descriptor();
        return { _descr.get_instance(), typeid(TmxStringEncoder), "string" };
	}

    bool is_binary() const noexcept override {
        return false;
    }

    template <typename _Tp>
    TmxError handle(_Tp const &obj, byte_stream &os) const {
        os << (typename _Tp::value_type)obj;
        return { };
    }

    TmxError handle(Null const &obj, byte_stream &os) const {
        os << "null";
        return { };
    }

    TmxError handle(Boolean const &obj, byte_stream &os) const {
        os << (obj ? "true" : "false");
        return { };
    }

    template <std::uint8_t _Sz>
    TmxError handle(String<_Sz> const &obj, byte_stream &os) const {
        os << to_char_sequence<typename byte_stream::char_type>(obj.c_str());
        return { };
    }

    template <typename _E>
    TmxError handle(Enum<_E> const &obj, byte_stream &os) const {
        os << obj.get_enum_name();
        return { };
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
//                        ptr = tmx::common::any_cast<std::shared_ptr<byte_stream> >(&args[1]);
//
//                    if (ptr && ptr->get())
//                        return this->handle(make_type(v), *(ptr->get()));
//
//                    return { EINVAL, "Invalid argument: Missing output stream to encode to." };
//                }
//        }, var, args);
//        return { };
//    }
};

class TmxStringDecoder: public TmxDecoder {
public:
	TmxStringDecoder() {
        this->register_decoder();
    }

	TmxTypeDescriptor get_descriptor() const noexcept override {
        static const auto &_descr = TmxDecoder::get_descriptor();
		return { _descr.get_instance(), typeid(TmxStringDecoder), "string" };
	}

    bool is_binary() const noexcept override {
        return false;
    }

    template <typename _Tp, typename _CharT>
    TmxError handle(_Tp const &type, char_sequence<_CharT> const &str, Any &val) const {
        auto &data = val.emplace<_Tp>();

        std::basic_istringstream<_CharT> is { str.data() };
        is >> *data;
        return { };
    }

    template <typename _CharT>
    TmxError handle(Null const &, char_sequence<_CharT> const &str, Any &val) const {
        val.emplace<Null>();
        return { };
    }

    template <std::uint8_t _Sz, typename _CharT>
    TmxError handle(String<_Sz> const &, char_sequence<_CharT> const &str, Any &val) const {
        if (str == "null")

        val.emplace< String<_Sz> >(str.data());
        return { };
    }

    template <typename _E, typename _CharT>
    TmxError handle(Enum<_E> const &, char_sequence<_CharT> const &str, Any &val) const {
        if (str.length() > 0 && ::isdigit(str[0])) {
            auto ret = this->handle(_E(), str, val);
            if (ret) return ret;

            _E *num = tmx::common::any_cast<_E>(&val);
            if (num)
                val.emplace< Enum<_E> >(*num);
            else
                val.emplace< Enum<_E> >(0);

            return { };
        } else {
            auto ret = this->handle(string_type<_CharT>(), str, val);
            if (ret) return ret;

            // TODO: Need an enumeration schema
            return { };
        }
    }

    TmxError execute(TmxTypeDescriptor const &type, std::reference_wrapper<TmxArgList> args) const override {
//        if (!type) {
           // Need an automatic conversion. Try to defer to the JSON encoder
            auto enc = codec::TmxDecoder::get_decoder("json");
            if (enc)
                return enc->execute(type, args);
//        }

//        TmxVariant<TmxScalarTypes, String8> var;
//
//        auto ret = set_variant(var, type ? type : TmxTypeRegistry().get(typeid(String<8>)));
//        if (!ret) return ret;
//
//        return do_handle(TmxTypeHandlerOverload {
//                [this](auto &v, TmxArgList &args) -> TmxError {
//                    byte_sequence *bytes = nullptr;
//                    if (args.size() > 0)
//                        bytes = tmx::common::any_cast<byte_sequence>(&args[0]);
//
//                    if (!bytes)
//                        return { EINVAL, "Invalid argument: No byte sequence to decode from." };
//
//                    return this->handle(make_type(v), to_char_sequence(*bytes), args.emplace_back());
//                }
//        }, var, args);
        return { };
    }
};

static TmxTypeRegistrar< TmxStringEncoder > _string_encoder_registrar;
static TmxTypeRegistrar< TmxStringDecoder > _string_decoder_registrar;

} /* End namespace codec */
} /* End namespace message */
} /* End namespace tmx */

