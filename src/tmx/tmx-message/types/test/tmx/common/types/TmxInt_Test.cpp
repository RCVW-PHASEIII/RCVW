/*!
 * Copyright (c) 2021 Battelle Memorial Institute
 *
 * All Rights Reserved.
 *
 * @file TmxInt_Test.cpp
 *
 *  Created on: Jun 21, 2021
 *      @author: Gregory M. Baumgardner
 */

#include <tmx/common/types/Int.hpp>

#include <boost/test/unit_test.hpp>

#include <bitset>
#include <chrono>
#include <cstdint>
#include <cmath>
#include <iostream>
#include <random>
#include <type_traits>

using namespace tmx::common::types;

struct IntGenerator {
	static auto &_instance() {
		static std::mt19937_64 _generator;
		static bool _init = true;

		if (_init) {
			::srand(std::chrono::seconds(std::time(NULL)).count());
			uint64_t seed = ((uint64_t)::rand() << 32) | ::rand();
			_generator.seed(seed);
			_init = false;
		}

		return _generator;
	}

	template <typename _T>
	static _T number() {
		static std::uniform_int_distribution<_T> dist;
		return dist(_instance());
	}
};

template <typename _T, std::size_t _Bytes, std::size_t _Bits>
void check_int_type() {

	static constexpr bool _signed = std::is_signed<_T>::value;

	typedef typename std::conditional<_signed, Int<_Bits>, UInt<_Bits> >::type test_type;
	typedef typename test_type::value_type test_type_t;

	test_type test1;

	//BOOST_TEST_MESSAGE("Testing " << tmx::common::type_short_name(test1) << " (" <<
	//		tmx::common::type_short_name<test_type_t>() << ") sanity.");

	std::string typeName { _signed ? "Int" : "UInt" };
	typeName.append("<");
	typeName.append(std::to_string(_Bits));
	typeName.append(">");

    BOOST_CHECK_EQUAL(TmxTypeTraits<decltype(test1)>::name, typeName);

	BOOST_CHECK_EQUAL(test1.numBytes, _Bytes);
	BOOST_CHECK_EQUAL(test1.numBits, _Bits);

    // The template value _T is one of the native types, std::int8_t, for example
    // The implementation of limits is calling std::numeric_limits<_T>.
    // Effectively, the min of -128 and max of 128 (signed char) is being used for a
    // 1-bit integer, which should have a min of 0, and a max of 1

    // Comments in int.hpp indicate this is by design, actual storage is the next larger
    // native type.
    // If so, then if not for space reduction, what is the point of having "non-native" sized
    // instances ?
	BOOST_CHECK_EQUAL(test1.limits().min(), std::numeric_limits<_T>::min());
	BOOST_CHECK_EQUAL(test1.limits().max(), std::numeric_limits<_T>::max());

	BOOST_CHECK_EQUAL(test1, 0);

	static uint64_t test_val1 = 0xAAAAAAAAAAAAAAAA;
    // 0XA = 1010 -> Odd bits of oddBitsOne = 1, Even bits = 0
    static uint64_t test_val2 = 0x5555555555555555;
    // 0x5 = 0101 -> Even bits of evenBitsOne = 1, Odd bits = 0
	static uint64_t test_val3 = 0x0012489ACDEFEC80;
	static uint64_t test_val4 = 0xFFFFFFFFFFFFFFFF;

	std::bitset<_Bits> oddBitsOne(test_val1);
	std::bitset<_Bits> evenBitsOne(test_val2);
	std::bitset<_Bits> test_bits3(test_val3);
	std::bitset<_Bits> allBitsOne(test_val4);

	uint64_t mask = allBitsOne.to_ullong();
    // mask is the value created from _Bits "1" bits, thus, _Bits = "2", mask = "11" = 0x03, _Bits = "3", mask = "111" = 0x07

    // I checked, this is properly taking the _Bits least significant bits
	test1 = static_cast<test_type_t>(test_val1);

	BOOST_CHECK_EQUAL(mask, test1.mask);

if ( 3 == _Bits )
    printf("Hello world");

    // What does this implicit cast do ??
	test_type_t negative = ~(mask);
    // ~mask flips 64 bits of mask, causing ls _Bits that were 1 to be "0"
    // THEN WHY isn't negative = 0 ????
    // e.g. _Bits = 2, _Bytes = 1, mask = 0x03, viewing memory in debugger shows this
    // ~(mask) least significant byte = 0xFC because ~0x03 = 1111 1100 (0x03) = 0000 0011
    // e.g. _Bits = 3, _Bytes = 1, mask = 0x07
    // ~(mask) least significant byte = 0xF8 because ~0x07 = 1111 1000 (0x07) = 0000 0111
    // So WHY isn't negative = 0 ????
    // Because the debugger doesn't know how to display test_type_t (?!?!)
    // later: because the MOST significant bit of the value (of test_type_t) will always be 0, and therefore
    // it flips to 1. so for ALL "signed" types up to 7 _Bits, negative will always be < 0

    // IT HAS to be because the "value" of a test_type_t does NOT honor just those bits in it
    // THAT is what is throwing me off. 

    if ( _signed ) {

        // The value _signed comes from the native type, so it will always be
        // reliable.
        // However, the implementation of is_signed for our class is as follows

        test_type_t loVal(-1);
        test_type_t hiVal(0);

        BOOST_CHECK_LT(loVal, hiVal);

    }

    if (  _signed && oddBitsOne[_Bits - 1] ) {
        BOOST_CHECK_EQUAL(true, test1 < 0);
        // | operation RS = rightmost _Bits of ...101010101010
        // | operation LS = _Bits of 0
        // Doesn't the ! operation ALWAYS return the RS ?
        BOOST_CHECK_EQUAL(negative | (test_type_t) (oddBitsOne . to_ullong()), test1);
    } else {
        BOOST_CHECK_EQUAL(oddBitsOne . to_ullong(), test1);
    }

	test_type test2(static_cast<test_type_t>(test_val2));

	if (_signed && evenBitsOne[_Bits - 1]) {
		BOOST_CHECK_EQUAL(true, test2 < 0);
		BOOST_CHECK_EQUAL(negative | (test_type_t)evenBitsOne.to_ullong(), test2);
	} else {
		BOOST_CHECK_EQUAL(evenBitsOne.to_ullong(), test2);
	}

	test_type_t x = static_cast<test_type_t>(test_val3);
	test_type test3(x);
	if (_signed && test_bits3[_Bits-1]) {
		BOOST_CHECK_EQUAL(true, test3 < 0);
        //NTC: I'm not sure why the following test WAS against *test3. It doesn't make sense to
        // treat an integer as a pointer. I have to assume it was a typo and
        // have removed the *
		BOOST_CHECK_EQUAL(negative | x, test3);
		BOOST_CHECK_EQUAL(negative | (test_type_t)test_bits3.to_ullong(), test3);
	} else {
		BOOST_CHECK_EQUAL(x & mask, (test_type_t)test3);
		BOOST_CHECK_EQUAL(test_bits3.to_ullong(), test3);
	}

	x = static_cast<test_type_t>(test_val4);
	if (_signed && allBitsOne[_Bits - 1]) {
		BOOST_CHECK_EQUAL(true, test3 < 0);
		BOOST_CHECK_EQUAL(test3, negative | x);
		BOOST_CHECK_EQUAL(negative | (test_type_t)allBitsOne.to_ullong(), test3);
	} else {
		BOOST_CHECK_EQUAL(test3, x & mask);
		BOOST_CHECK_EQUAL(allBitsOne.to_ullong(), test3);
	}

	test_type test5;
	BOOST_CHECK_NE(test3, test5);
	//BOOST_CHECK_NE(test5, x);
	//BOOST_CHECK_EQUAL(false, (test3 == test5));
	//BOOST_CHECK_EQUAL(true, (test3 != test5));

	test5 = x;
	BOOST_CHECK_EQUAL(test3, test5);
	//BOOST_CHECK_EQUAL(true, (test3 == test5));
	//BOOST_CHECK_EQUAL(false, (test3 != test5));
	if (_signed && allBitsOne[_Bits - 1])
		BOOST_CHECK_EQUAL(test3, negative | x);
	else
		BOOST_CHECK_EQUAL(test3, x & mask);

	// Try 25 random integers
	for (size_t i = 0; i < 25; i++) {

		test_type_t test_val5 = IntGenerator::number<test_type_t>();

        std::bitset<_Bits> test_bits5(test_val5);

		test5 = test_val5;
		if (_signed && test_bits5[_Bits-1]) {
			BOOST_CHECK_EQUAL(true, test5 < 0);
			BOOST_CHECK_EQUAL(negative | (test_type_t)test_bits5.to_ullong(), test5);
		} else {
			BOOST_CHECK_EQUAL(test_bits5.to_ullong(), test5);
		}
	}

	check_int_type<_T, _Bytes, _Bits+1>();
}

template <>
void check_int_type<std::int8_t, 1, 9>() { }

template <>
void check_int_type<std::uint8_t, 1, 9>() { }

template <>
void check_int_type<std::int16_t, 2, 17>() { }

template <>
void check_int_type<std::uint16_t, 2, 17>() { }

template <>
void check_int_type<std::int32_t, 4, 33>() { }

template <>
void check_int_type<std::uint32_t, 4, 33>() { }

template <>
void check_int_type<std::int64_t, 8, 65>() { }

template <>
void check_int_type<std::uint64_t, 8, 65>() { }

BOOST_AUTO_TEST_CASE( test_int8_type ) {
	static constexpr bool b =
			std::is_same<typename tmx::common::types::Intmin::value_type, std::int8_t>::value;
	BOOST_CHECK_EQUAL(true, b);
	BOOST_CHECK_EQUAL(8, tmx::common::types::Intmin::numBits);
	check_int_type<std::int8_t, 1, 1>();
}

BOOST_AUTO_TEST_CASE( test_int16_type ) {
	check_int_type<std::int16_t, 2, 9>();
}

BOOST_AUTO_TEST_CASE( test_int32_type ) {
	static constexpr bool b =
			std::is_same<typename tmx::common::types::Int_::value_type, std::int32_t>::value;
	BOOST_CHECK_EQUAL(true, b);
	BOOST_CHECK_EQUAL(32, tmx::common::types::Int_::numBits);
	check_int_type<std::int32_t, 4, 17>();
}

BOOST_AUTO_TEST_CASE( test_int64_type ) {
	static constexpr bool b =
			std::is_same<typename tmx::common::types::Intmax::value_type, std::int64_t>::value;
	BOOST_CHECK_EQUAL(true, b);
	BOOST_CHECK_EQUAL(64, tmx::common::types::Intmax::numBits);
	check_int_type<std::int64_t, 8, 33>();
}

BOOST_AUTO_TEST_CASE( test_uint8_type ) {
	static constexpr bool b =
			std::is_same<typename tmx::common::types::UIntmin::value_type, std::uint8_t>::value;
	BOOST_CHECK_EQUAL(true, b);
	BOOST_CHECK_EQUAL(8, tmx::common::types::UIntmin::numBits);
	check_int_type<std::uint8_t, 1, 1>();
}

BOOST_AUTO_TEST_CASE( test_uint16_type ) {
	check_int_type<std::uint16_t, 2, 9>();
}

BOOST_AUTO_TEST_CASE( test_uint32_type ) {
	static constexpr bool b =
			std::is_same<typename tmx::common::types::UInt_::value_type, std::uint32_t>::value;
	BOOST_CHECK_EQUAL(true, b);
	BOOST_CHECK_EQUAL(32, tmx::common::types::UInt_::numBits);
	check_int_type<std::uint32_t, 4, 17>();
}

BOOST_AUTO_TEST_CASE( test_uint64_type ) {
	static constexpr bool b =
			std::is_same<typename tmx::common::types::UIntmax::value_type, std::uint64_t>::value;
	BOOST_CHECK_EQUAL(true, b);
	BOOST_CHECK_EQUAL(64, tmx::common::types::UIntmax::numBits);
	check_int_type<std::uint64_t, 8, 33>();
}
