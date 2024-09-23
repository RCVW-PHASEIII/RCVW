/*!
 * Copyright (c) 2021 Battelle Memorial Institute
 *
 * All Rights Reserved.
 *
 * @file TmxStringEncoder.cpp
 *
 *  Created on: Aug 3, 2021
 *      @author: gmb
 */

#include <tmx/message/codec/TmxCodec.hpp>

#include <tmx/common/types/support/introspect.hpp>

#include <iomanip>
#include <sstream>

using namespace tmx::common;
using namespace tmx::common::types;

namespace tmx {
namespace message {
namespace codec {

class TmxStringEncoder: public TmxCodec {
public:
	typedef string8 type;

	explicit TmxStringEncoder(): TmxCodec() { }
	explicit TmxStringEncoder(bool): TmxCodec(this) { }

	using TmxTypeVisitor::dispatch;

	std::error_code encode(const any &data, byte_stream &theBytes) const override {
		// Need a new instance to work with
		auto enc = new TmxStringEncoder();
		auto ret = enc->dispatch(data);
		if (!ret) {
			for (auto i = 0; i < enc->_stream.str().length(); i++)
				theBytes.push_back((byte_t) enc->_stream.str()[i]);
		}

		return ret;
	}

	std::error_code decode(const byte_stream &theBytes, any &data) const override {
		types::bytes_type<typename types::string::char_t> _bytes { theBytes.data(), theBytes.size() };
		data = _bytes.to_string();
		return std::error_code();
	}

	string::view_type get_name() const override {
		return "string";
	}

	std::error_code ec(const typename type::char_t *val) {
		this->_stream << val;
		return std::error_code();
	}

	template <typename _Val>
	std::error_code ec(_Val val) {
		return ec(std::to_string(val).c_str());
	}

	template <bitcount_type _FSz>
	std::error_code ec(const Float<_FSz> &val) {
		typedef std::numeric_limits<typename Float<_FSz>::type> type;
		std::stringstream ss;
		ss << std::setprecision(type::max_digits10) << val;
		return ec(ss.str().c_str());
	}

	std::error_code dispatch(const null_type &) override {
		return ec("null");
	}
	std::error_code dispatch(const boolean_type &b) override {
		return ec(*b ? "true" : "false");
	}
	std::error_code dispatch(const uint1 &i) override {
		return ec(*i ? 1 : 0);
	}
	std::error_code dispatch(uint64 &&i, bitcount_type bits) override {
		return ec(*i);
	}
	std::error_code dispatch(int64 &&i, bitcount_type bits) override {
		return ec(*i);
	}
	std::error_code dispatch(const float32 &f) override {
		return ec<32>(f);
	}
	std::error_code dispatch(const float64 &f) override {
		return ec<64>(f);
	}
#ifdef TMX_FLOAT128
	std::error_code dispatch(const float128 &f) override {
		return ec<128>(f);
	}
#endif
	std::error_code dispatch(const string8 &s) override {
		return ec(s.c_str());
	}
	std::error_code dispatch(const array_type<any> &a) override {
		// Convert to a CSV output
		for (auto &val: a) {
			if (this->_stream.str().length())
				this->_stream << ',';

			this->dispatch(val);
		}

		return std::error_code();
	}

	std::error_code dispatch(const map_type<string, any> &m) override {
		// Convert to a properties output
		for (auto &val: m) {
			if (this->_stream.str().length())
				this->_stream << '\n';

			this->_stream << val.first << '=';
			this->dispatch(val.second);
		}

		return std::error_code();
	}

	std::stringstream _stream;
};

static TmxStringEncoder _string_encoder { true };

} /* End namespace codec */
} /* End namespace message */
} /* End namespace tmx */


