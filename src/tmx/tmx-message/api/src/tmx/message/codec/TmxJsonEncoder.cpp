/*!
 * Copyright (c) 2021 Battelle Memorial Institute
 *
 * All Rights Reserved.
 *
 * @file TmxJsonEncoder.cpp
 *
 *  Created on: Aug 16, 2021
 *      @author: gmb
 */

#include <tmx/message/codec/TmxCodec.hpp>

#define BOOST_JSON_STANDALONE
#include <tmx/common/types/support/thirdparty/boost/json.hpp>
#include <tmx/common/types/support/thirdparty/boost/json/src.hpp>

using namespace tmx::common;
using namespace tmx::common::types;

namespace tmx {
namespace message {
namespace codec {

std::error_code any_to_json(const any &, boost::json::value &);
std::error_code json_to_any(const boost::json::value &, any &);

class TmxJsonEncoder: public TmxCodec {
public:
	explicit TmxJsonEncoder(): TmxCodec() { }
	explicit TmxJsonEncoder(bool): TmxCodec(this) { }

	typename string8::view_type get_name() const {
		return "json";
	}

	using TmxTypeVisitor::dispatch;

	std::error_code encode(const any &data, byte_stream &theBytes) const override {
		boost::json::value json;
		auto ret = any_to_json(data, json);
		if (!ret) {
			auto str = boost::json::serialize(json);
			for (auto i = 0; i < str.length(); i++)
				theBytes.push_back((byte_t) str[i]);
		}

		return ret;
	}

	std::error_code decode(const byte_stream &theBytes, any &data) const override {
		std::error_code _ec;
		types::bytes_type<typename types::string::char_t> _bytes { theBytes.data(), theBytes.size() };

		auto dec = new TmxJsonEncoder();
		dec->_value = boost::json::value(boost::json::parse(_bytes.to_string(), _ec));

		if (!_ec) {
			// TODO: Do the decode
			auto ret = json_to_any(dec->_value, data);
			if (ret)
				return ret;
		}

		delete dec;
		return _ec;
	}

	template <typename _T>
	std::error_code ec(_T &&val) {
		this->_value = val;
		return std::error_code();
	}

	std::error_code dispatch(const null_type &n) override {
		return ec(n.value);
	}

	std::error_code dispatch(const boolean_type &b) override {
		return ec(*b);
	}

	std::error_code dispatch(uint64 &&i, bitcount_type bits) override {
		return ec(*i);
	}

	std::error_code dispatch(int64 &&i, bitcount_type bits) override {
		return ec(*i);
	}

	std::error_code dispatch(const float32 &f) override {
		return ec(*f);
	}

	std::error_code dispatch(const float64 &f) override {
		return ec(*f);
	}

#ifdef TMX_FLOAT128
	std::error_code dispatch(const float128 &f) override {
		// Only up to double is supported
		return ec(static_cast<typename float64::type>(*f));
	}
#endif

	std::error_code dispatch(const string8 &s) override {
		return ec((typename string8::type) s);
	}

	std::error_code dispatch(const array_type<TmxData> &a) override {
		boost::json::array val;
		for (auto item: a) {
			auto ret = this->dispatch(item);
			if (ret)
				return ret;

			val.push_back(this->_value);
		}

		this->_value = val;
		return std::error_code();
	}

	std::error_code dispatch(const map_type<string8, TmxData> &m) override {
		boost::json::object val;
		for (auto item: m) {
			auto ret = this->dispatch(item.second);
			if (ret)
				return ret;

			val[(typename string8::type) item.first] = this->_value;
		}

		this->_value = val;
		return std::error_code();
	}

	boost::json::value _value;
};

std::error_code any_to_json(const any &data, boost::json::value &json) {
	auto enc = new TmxJsonEncoder();
	auto ret = enc->dispatch(data);
	if (!ret)
		json = enc->_value;

	delete enc;
	return ret;
}

std::error_code json_to_any(const boost::json::value &json, any &data) {
	if (json.is_null())
		return std::error_code();

	auto bptr = json.if_bool();
	if (bptr) {
		data = *bptr;
		return std::error_code();
	}

	auto iptr = json.if_int64();
	if (iptr) {
		data = int64(*iptr);
		return std::error_code();
	}

	auto uptr = json.if_uint64();
	if (uptr) {
		data = uint64(*uptr);
		return std::error_code();
	}

	auto dptr = json.if_double();
	if (dptr) {
		data = double_type(*dptr);
		return std::error_code();
	}

	auto sptr = json.if_string();
	if (sptr) {
		data = sptr->c_str();
		return std::error_code();
	}

	auto aptr = json.if_array();
	if (aptr) {
		for (size_t i = 0; i < aptr->size(); i++) {
			auto ret = json_to_any(aptr->at(i), data[i]);
			if (ret) return ret;
		}
		return std::error_code();
	}

	auto mptr = json.if_object();
	if (mptr) {
		for (auto item: *mptr) {
			auto ret = json_to_any(item.value(), data[item.key().data()]);
			if (ret) return ret;
		}
	}

	// TODO: Something wrong?
	return std::error_code();
}



static TmxJsonEncoder _json_encoder { true };

}
}
}

