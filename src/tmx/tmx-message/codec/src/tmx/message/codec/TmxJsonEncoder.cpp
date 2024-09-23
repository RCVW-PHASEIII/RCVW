/*!
 * Copyright (c) 2021 Battelle Memorial Institute
 *
 * All Rights Reserved.
 *
 * @file TmxJsonEncoder.cpp
 *
 *  Created on: Aug 16, 2021
 *      @author: Gregory M. Baumgardner
 */

#include <tmx/platform.hpp>

#include <tmx/common/TmxLogger.hpp>
#include <tmx/message/codec/TmxCodec.hpp>

#include <iomanip>

#define BOOST_JSON_STANDALONE
#include <tmx/message/codec/thirdparty/boost/json.hpp>
#include <tmx/message/codec/thirdparty/boost/json/src.hpp>

using namespace tmx::common;
using namespace tmx::common::types;

namespace tmx {
namespace message {
namespace codec {

class TmxJsonEncoder: public TmxEncoder {
public:
	TmxJsonEncoder() {
        this->register_encoder();
    }

	TmxTypeDescriptor get_descriptor() const noexcept override {
		static const auto &_descr = TmxEncoder::get_descriptor();
        return { _descr.get_instance(), typeid(TmxJsonEncoder), "json" };
	}

    bool is_binary() const noexcept override {
        return false;
    }

    template <typename _Tp>
    TmxError handle(_Tp const &obj, byte_stream &os) const {
        os << boost::json::value((typename _Tp::value_type)obj);
        return { };
    }

    TmxError handle(Null const &obj, byte_stream &os) const {
        os << boost::json::value();
        return { };
    }

    template <std::uint8_t _Sz>
    TmxError handle(Float<_Sz> const &obj, byte_stream &os) const {
        os << boost::json::value(static_cast<Double::value_type>((typename Float<_Sz>::value_type)obj));
        return { };
    }

    template <std::uint8_t _Sz>
    TmxError handle(String<_Sz> const &obj, byte_stream &os) const {
        // Special case:
        // If the string is already a JSON map or array or a string, the just write it out
        auto &firstChar = obj.front();
        auto &lastChar = obj.back();
        if (obj.length() >= 1 && ((firstChar == '{' && lastChar == '}') ||
                (firstChar == '[' && lastChar == ']')) ||
                (firstChar == '"' && lastChar == '"'))
            os << obj.c_str();
        else
            os << boost::json::value(to_char_sequence<typename byte_stream::char_type>(obj.c_str()));
        return { };
    }

    template <typename _E>
    TmxError handle(Enum<_E> const &obj, byte_stream &os) const {
        if (obj.get_enum_name().length() > 0 && !::isdigit(obj.get_enum_name()[0]))
            return this->handle(String8(obj.get_enum_name()), os);
        else
            return this->handle(UIntmax(obj.get_enum_value()), os);
    }

    TmxError handle(Any const &obj, byte_stream &os) const {
        return this->encode(obj, os);
    }

    template <typename _Tp>
    TmxError handle(Array<_Tp> const &arr, byte_stream &os) const {
        os << "[";
        for (std::size_t i = 0; i < arr.size(); i++){
            if (i > 0)
                os << ", ";

            this->handle(arr[i], os);
        }
        os << "]";

        return { };
    }

    template <typename _Key, typename _Tp>
    TmxError handle(Map<_Key, _Tp> const &props, byte_stream &os) const {
        bool first = true;
        os << "{";
        for (auto &obj: props) {
            if (!first)
                os << ", ";

            this->handle(obj.first, os);
            os << ":";
            this->handle(obj.second, os);
            first = false;
        }
        os << "}";

        return { };
    }

    TmxError execute(TmxTypeDescriptor const &type, std::reference_wrapper<TmxArgList> args) const override {
        TmxVariant<TmxScalarTypes, String8, TmxBasicComplexTypes> var;

        static TmxTypeHandlerOverload handler{
                [this](auto const &v, TmxArgList &args) -> TmxError {
                    std::shared_ptr<byte_stream> *ptr;
                    if (args.size() > 1)
                        ptr = tmx::common::any_cast<std::shared_ptr<byte_stream> >(&args[1]);

                    if (ptr && ptr->get())
                        return this->handle(make_type(v), *(ptr->get()));

                    return { EINVAL, "Invalid argument: Missing output stream to encode to." };
                }
        };

        auto err = set_variant(var, type);
        if (!err)
            return do_handle(handler, var, args);

        // Special support for the TmxMessage bytes type
        if (err.get_code() == ENOTSUP && args.get().size() > 0) {
            auto try1 = common::types::as< std::basic_string<tmx::common::byte_t> >((args.get())[0]);
            if (try1)
                return handler(common::to_char_sequence(try1->data(), try1->length()), args);
        }

        return err;
    }
};

class TmxJsonDecoder: public TmxDecoder {
public:
	TmxJsonDecoder() {
        this->register_decoder();
    }

	TmxTypeDescriptor get_descriptor() const noexcept override {
        static const auto &_descr = TmxDecoder::get_descriptor();
		return { _descr.get_instance(), typeid(TmxJsonDecoder), "json" };
	}

    bool is_binary() const noexcept override {
        return false;
    }

    TmxError execute(TmxTypeDescriptor const &type, std::reference_wrapper<TmxArgList> args) const override {
        byte_sequence *bytes = nullptr;
        if (args.get().size() > 0)
            bytes = tmx::common::any_cast<byte_sequence>(&(args.get())[0]);

        if (!bytes)
            return { EINVAL, "Invalid argument: No byte sequence to decode from." };

        TLOG(DEBUG2) << "Parsing " << bytes->length() << " bytes. (" << *bytes << ")";

        // Trim the input of white space
        auto chars = trim(to_char_sequence(*bytes));
		std::error_code _ec;

        auto &data = args.get().emplace_back();

        // This may just be a plain-old string, in which case we do not want to parse as it will cause an error
        TmxVariant<String8> _strVar;
        auto ret = set_variant(_strVar, TmxTypeRegistry().get("String8"));
        if (!ret) {
            if (chars.length() && chars.front() != '{' && chars.front() != '[')
                return this->json_to_any(boost::json::string(chars), data);
        }

		auto json = boost::json::value(boost::json::parse(chars, _ec));

		if (_ec) {
            TmxError ret { _ec };
            ret[std::string("message")] = ret.get_message() + ": " +
                    std::string((const char *)bytes->data(), bytes->length());
            return ret;
        }

		return this->json_to_any(json, data);
	}

	TmxError json_to_any(const boost::json::value &json, types::Any &data) const {
		if (json.is_null()) {
            data = Null();
            return { };
        }

		auto bptr = json.if_bool();
		if (bptr) {
			data = make_any(*bptr);
			return { };
		}

		auto iptr = json.if_int64();
		if (iptr) {
			data = make_any(*iptr);
			return { };
		}

		auto uptr = json.if_uint64();
		if (uptr) {
			data = make_any(*uptr);
			return { };
		}

		auto dptr = json.if_double();
		if (dptr) {
			data = make_any(*dptr);
			return { };
		}

		auto sptr = json.if_string();
		if (sptr) {
			data = make_any(sptr->c_str());
			return { };
		}

		auto aptr = json.if_array();
		if (aptr) {
			typedef Array<types::Any> array_type;

            auto &_array = data.emplace<array_type>();
            _array.resize(aptr->size());

			for (size_t i = 0; i < aptr->size(); i++) {
				auto ret = json_to_any(aptr->at(i), _array.at(i));
				if (ret) return ret;
			}

			return { };
		}

		auto mptr = json.if_object();
		if (mptr) {
            typedef types::Properties<types::Any> properties_type;
            typedef typename properties_type::key_t key_type;

            properties_type &_props = data.emplace<properties_type>();
			_props.reserve(mptr->size());

			for (auto item: *mptr) {
				_props->operator[](key_type(item.key())) = Null();
				auto ret = json_to_any(item.value(), _props.at(key_type(item.key())));
				if (ret) return ret;
			}

			return { };
		}

		// TODO: Something wrong?
		data = Null();
		return { EINVAL, "Unknown JSON type supplied. Cannot decode." };
	}
};

static TmxTypeRegistrar< TmxJsonEncoder > _json_encoder;
static TmxJsonDecoder _json_decoder;

} /* End namespace codec */
} /* End namespace message */
} /* End namespace tmx */

