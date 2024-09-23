/*
 * RTCMTypes.hpp
 *
 *  Created on: Apr 26, 2018
 *      Author: gmb
 */

#ifndef INCLUDE_RTCM_RTCMTYPES_H_
#define INCLUDE_RTCM_RTCMTYPES_H_

#include <tmx/common/types/Int.hpp>

#include <algorithm>
#include <bitset>
#include <type_traits>

namespace tmx {
namespace messages {
namespace rtcm {

typedef common::types::UInt1 bit1;
typedef common::types::UInt2 bit2;
typedef common::types::UInt3 bit3;
typedef common::types::UInt4 bit4;

template <typename RtcmTraits>
typename RtcmTraits::data_type addSign(typename RtcmTraits::data_type value) {
	// Convert a negative with two's complement
	if (RtcmTraits::is_signed) {
		if (value < 0)
			return (typename RtcmTraits::data_type)(typename RtcmTraits::bitset(value).to_ulong());
		else if ((value >> (RtcmTraits::size - 1)))
			return -1 * ((~value + 1) & RtcmTraits::bitmask());
	}

	return value;
}

template <typename RtcmTraits, typename T>
T rollByte(T byte) {
	std::bitset<RtcmTraits::size> val;
	for (size_t i = 0; i < val.size(); i++) {
		size_t shift = val.size() - i - 1;
		val[i] = (byte & (1 << shift)) == (1 << shift);
	}

	return (T) val.to_ulong();
}

template <typename RtcmTraits, typename Word, typename T = uintmax_t>
T combineWord(Word word, T value) {
	if (value > 0) value <<= RtcmTraits::size;
	value |= (word & RtcmTraits::bitmask());
	return value;
}

template <typename RtcmTraits, typename Word, typename T = uintmax_t>
size_t splitWord(std::vector<Word> &words, T value, size_t pos) {
	words.push_back((Word)((value >> (RtcmTraits::size * pos)) & RtcmTraits::bitmask()));
	return 1;
}

template <typename RtcmTraits, char N>
tmx::byte_t get_Bit(const typename RtcmTraits::data_type &word) {
	static_assert(N <= RtcmTraits::size, "There are not that many bits in the RTCM word.");
	static constexpr char bit = RtcmTraits::size - N;
	return (word >> bit) & 0x01;
}

template <char Bit, char... OtherBits>
struct bit_manipulator {
	template <typename RtcmTraits>
	static tmx::byte_t xor_bits(const typename RtcmTraits::data_type &word) {
		return get_Bit<RtcmTraits, Bit>(word) ^
				bit_manipulator<OtherBits...>::template xor_bits<RtcmTraits>(word);
	}
};

template <char Bit>
struct bit_manipulator<Bit> {
	template <typename RtcmTraits>
	static tmx::byte_t xor_bits(const typename RtcmTraits::data_type &word) {
		return get_Bit<RtcmTraits, Bit>(word);
	}
};

} /* End namespace rtcm */
} /* End namespace messages */
} /* End namespace tmx */


// The attribute builder macros used for these types
#define typesafe_rtcm_getter_builder(C, X, Y, G) \
	typename X::data_type attr_func_name(G, Y)() { \
		return attr_field_name(Y).get(C); \
	} \
	typename X::bitset attr_func_name(G, Y ## Bits)() { \
		return typename X::bitset(attr_func_name(G, Y)()); \
	}
#define typesafe_rtcm_setter_builder(C, X, Y, S, L) \
	void attr_func_name(S, Y)(const typename X::data_type value) { \
		L { \
			attr_field_name(Y).set(C, value); \
		} \
	} \
	void attr_func_name(S, Y ## Bits)(const typename X::bitset bits) { \
		attr_func_name(S, Y)((typename X::data_type)bits.to_ulong()); \
	}
#define rtcm_attribute_builder(T, X, Y, D) \
public: \
	struct Y: public X { \
		static data_type default_value() { return D; } \
	}; \
private: \
	T attr_field_name(Y);
#define ro_rtcm_attribute(C, T, X, Y, G, D) \
	rtcm_attribute_builder(T, X, Y, D) \
public: \
	typesafe_rtcm_getter_builder(C, X, Y, G)
#define rw_rtcm_attribute(C, T, X, Y, G, D, S, L) \
	ro_rtcm_attribute(C, T, X, Y, G, D) \
	typesafe_rtcm_setter_builder(C, X, Y, S, L)
#define std_rtcm_attribute(C, X, Y, D, L) \
	rw_rtcm_attribute(C, battelle::attributes::standard_attribute<Y>, X, Y, get_, D, set_, L)

#endif /* INCLUDE_RTCM_RTCMTYPES_H_ */
