/*!
 * Copyright (c) 2021 Battelle Memorial Institute
 *
 * All Rights Reserved.
 *
 * @file TmxHexstringEncoder.cpp
 *
 *  Created on: Aug 3, 2021
 *      @author: gmb
 */

#include <tmx/message/codec/TmxCodec.hpp>

#include <tmx/common/types/support/introspect.hpp>
#include <tmx/common/types/support/byte_order.hpp>

using namespace boost::hana;
using namespace tmx::common;
using namespace tmx::common::types;

#include <bitset>
#include <cmath>
#include <string>
#include <sstream>

#include <iostream>

namespace tmx {
namespace message {
namespace codec {

template <typename TmxTypeVisitor::bitcount_type _Sz>
struct _characterset {
	static constexpr auto _capital_letters =   TMX_CHAR_TUPLE_STRING("ABCDEFGHIJKLMNOPQRSTUVWXYZ");
	static constexpr auto _lowercase_letters = TMX_CHAR_TUPLE_STRING("abcdefghijklmnopqrstuvwxyz");
	static constexpr auto _digits =            TMX_CHAR_TUPLE_STRING("0123456789");
};

template <>
struct _characterset<16> {
	static constexpr auto _base16_chars =
			_characterset<0>::_digits + string_c<
				_characterset<0>::_capital_letters[int_c<0>],
				_characterset<0>::_capital_letters[int_c<1>],
				_characterset<0>::_capital_letters[int_c<2>],
				_characterset<0>::_capital_letters[int_c<3>],
				_characterset<0>::_capital_letters[int_c<4>],
				_characterset<0>::_capital_letters[int_c<5>]
			>;

	static constexpr auto value = _base16_chars.c_str();
	static constexpr uint8_t bits = 4;
	static constexpr uint8_t size = 1;

	static inline auto htobe(typename uint8::type i) { return i; }
	static inline auto betoh(typename uint8::type i) { return i; }
};

template <>
struct _characterset<32> {
	static constexpr auto _base32_chars =
			_characterset<0>::_capital_letters + string_c<
				_characterset<0>::_digits[int_c<2>],
				_characterset<0>::_digits[int_c<3>],
				_characterset<0>::_digits[int_c<4>],
				_characterset<0>::_digits[int_c<5>],
				_characterset<0>::_digits[int_c<6>],
				_characterset<0>::_digits[int_c<7>]
			>;

	static constexpr auto value = _base32_chars.c_str();
	static constexpr uint8_t bits = _characterset<16>::bits + 1;
	static constexpr uint8_t size = _characterset<16>::size + 4;

	static inline auto htobe(typename uint32::type i) { return htobe32(i); }
	static inline auto betoh(typename uint32::type i) { return be32toh(i); }
};

template <>
struct _characterset<64> {
	static constexpr auto _base64_chars =
			_characterset<0>::_capital_letters +
			_characterset<0>::_lowercase_letters +
			_characterset<0>::_digits +
				string_c<'+', '/'>;

	static constexpr auto value = _base64_chars.c_str();
	static constexpr uint8_t bits = _characterset<32>::bits + 1;
	static constexpr uint8_t size = _characterset<32>::size - 2;

	static inline auto htobe(typename uint64::type i) { return htobe64(i); }
	static inline auto betoh(typename uint64::type i) { return be64toh(i); }
};

template <typename TmxTypeVisitor::bitcount_type _Sz>
class TmxByteEncoder: public TmxCodec {
	static constexpr auto _nm = support::concat(TMX_STATIC_STRING("base-"), support::make_string<_Sz>());

	static constexpr auto characters = _characterset<_Sz>::value;
	static constexpr auto bits = _characterset<_Sz>::bits;
	static constexpr auto size = _characterset<_Sz>::size;
	static constexpr auto numChars = 8 * size / bits;
public:
	using TmxTypeVisitor::dispatch;

	explicit TmxByteEncoder(): TmxCodec() { }
	explicit TmxByteEncoder(bool): TmxCodec(this) { }

	typename string8::view_type get_name() const {
		return _nm.c_str();
	}

	std::error_code encode(const any &data, byte_stream &theBytes) const override {
		static constexpr char _padding = '=';

		// Need a new instance to work with
		auto enc = new TmxByteEncoder<_Sz>();
		auto ret = enc->dispatch(data);
		if (!ret) {
			// Add last character
			if (enc->index < this->buf.size() - 1)
				enc->dump();

			// Append the padding characters
			auto n = enc->_stream.size() % numChars;
			if (n) n = numChars - n;
			while (n--) enc->_stream += _padding;

			for (auto i = 0; i < enc->_stream.length(); i++)
				theBytes.push_back((byte_t) enc->_stream[i]);
		}

		delete enc;
		return ret;
	}

	std::error_code decode(const byte_stream &theBytes, any &data) const override {
		types::bytes_type<typename types::string::char_t> _bytes { theBytes.data(), theBytes.size() };
		data = _bytes.to_string();
		return std::error_code();
	}

	void dump() {
		this->_stream += characters[this->buf.to_ulong() & UInt<bits>::mask];
		this->buf.reset();
		this->index = (typename UInt<bits>::type) this->buf.size() - 1;
	}

	void append(unsigned char bit) {
		if (this->index > bits)
			dump();

		this->buf[this->index] = (bit & 0x01);
		this->index--;
	}

	void append(const unsigned char *bytes, std::size_t bits) {
		// Do up to the last byte
		for (std::size_t i = 0; i < bits / 8; i++) {
			const unsigned char &val = bytes[i];
			append(val >> 7);
			append(val >> 6);
			append(val >> 5);
			append(val >> 4);
			append(val >> 3);
			append(val >> 2);
			append(val >> 1);
			append(val >> 0);
		}

		// Do the last byte, if needed
		for (std::size_t i = 0; i < (bits % 8); i++) {
			const unsigned char &val = bytes[(bits / 8) + 1];
			append(val >> (7 - i));
		}
	}

	void append(const char *bytes, std::size_t chars) {
		append((const unsigned char *)bytes, 8 * chars);
	}

	template <typename _T>
	union to_bytes {
		typename _T::type value;
		byte_t bytes[sizeof(value)];
	};

	template <typename _T>
	std::error_code ec(typename _T::type v, bitcount_type bits) {
		static constexpr auto _size = sizeof(_T);

		to_bytes<_T> _tb;
		_tb.value = v;

		// Reverse the bytes if the necessary
		if (!support::is_byte_order(TMX_NETWORK_BYTE_ORDER)) {
			for (std::size_t i = 0; _size - i - 1 > i; i++)
				std::swap(_tb.bytes[i], _tb.bytes[_size - i - 1]);
		}

		append(_tb.bytes + (_size - bits / 8), bits);
		return std::error_code();
	}

	std::error_code dispatch(const boolean_type &b) override {
		return ec<boolean_type>(*b, 1);
	}
	std::error_code dispatch(uint64 &&i, bitcount_type bits) override {
		return ec<uint64>(*i, bits);
	}
	std::error_code dispatch(int64 &&i, bitcount_type bits) override {
		return ec<int64>(*i, bits);
	}
	std::error_code dispatch(const float32 &f) override {
		return ec<float32>(*f, 32);
	}
	std::error_code dispatch(const float64 &f) override {
		return ec<float64>(*f, 64);
	}
#ifdef TMX_FLOAT128
	std::error_code dispatch(const float128 &f) override {
		return ec<float128>(*f, 128);
	}
#endif
	std::error_code dispatch(const string8 &s) override {
		append(s.c_str(), s.length());
		return std::error_code();
	}
	std::error_code dispatch(const string16 &s) override {
		return std::error_code();//ec(*s, 16 * s.length());
	}
	std::error_code dispatch(const string32 &s) override {
		return std::error_code();//ec(*s, 32 * s.length());
	}
	std::error_code dispatch(const wstring &s) override {
		return std::error_code();//ec(*s, support::bit_sizeof<typename wstring::char_t>() * s.length());
	}

	std::bitset<bits> buf { 0 };
	uint8_t index { bits - 1 };

	std::string _stream;

	unsigned char lastByte;
	bitcount_type bitIndex { 0 };
};

static TmxByteEncoder<16> _b16_encodec { true };
static TmxByteEncoder<32> _b32_encoder { true };
static TmxByteEncoder<64> _b64_encoder { true };

} /* End namespace codec */
} /* End namespace message */
} /* End namespace tmx */
