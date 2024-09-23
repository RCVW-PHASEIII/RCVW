/*!
 * Copyright (c) 2021 Battelle Memorial Institute
 *
 * All Rights Reserved.
 *
 * @file TmxFloat_Test.cpp
 *
 *  Created on: Jun 22, 2021
 *      @author: Gregory M. Baumgardner
 */

#include <tmx/common/types/Float.hpp>

#include <boost/test/unit_test.hpp>

#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <random>

using namespace tmx::common::types;

struct FloatGenerator {
	static auto &_instance() {
		static std::mt19937_64 _generator;
		static bool _init = true;

		if (_init) {
			std::srand(std::chrono::seconds(std::time(NULL)).count());
			uint64_t seed = ((uint64_t)std::rand() << 32) | std::rand();
			_generator.seed(seed);
			_init = false;
		}

		return _generator;
	}

	template <typename _T>
	static _T number() {
		static std::uniform_real_distribution<_T> dist;
		return dist(_instance());
	}
};

template <typename _T, std::size_t _Bytes, std::size_t _Bits>
void check_float_type() {
	typedef Float<_Bits> test_type;
	typedef typename test_type::value_type test_type_t;

	test_type test1;
	//BOOST_TEST_MESSAGE("Testing " << tmx::common::type_short_name(test1) << " (" <<
	//		tmx::common::type_short_name<test_type_t>() << ") sanity.");

	std::string typeName { "Float" };
	typeName.append("<");
	typeName.append(std::to_string(_Bits));
	typeName.append(">");
	BOOST_CHECK_EQUAL(TmxTypeTraits<decltype(test1)>::name, typeName);

	if (_Bits == 32)
		BOOST_CHECK_EQUAL(true, test1.template is_precision<float_precision::single>());
	else if (_Bits == 64)
		BOOST_CHECK_EQUAL(true, test1.template is_precision<float_precision::double_>());
	else if (_Bits == 128)
		BOOST_CHECK_EQUAL(true, test1.template is_precision<float_precision::quadruple>());

	BOOST_CHECK_EQUAL(test1.numBytes, _Bytes);
	BOOST_CHECK_EQUAL(test1.numBits, _Bits);
	BOOST_CHECK_EQUAL(test1.limits().min(), std::numeric_limits<_T>::min());
	BOOST_CHECK_EQUAL(test1.limits().max(), std::numeric_limits<_T>::max());
	BOOST_CHECK_EQUAL(test1, 0);

	static long double test_val1 = std::acos(-1);		// Pi
	static long double test_val2 = -1 * std::exp(1);	// -e
	static long double test_val3 = std::sqrt(45.891);

	test1 = static_cast<test_type_t>(test_val1);
	BOOST_CHECK_EQUAL((test_type_t)test_val1, test1);
	BOOST_TEST(std::cos(test1), -1);

	test_type test2(static_cast<test_type_t>(test_val2));
	BOOST_CHECK_EQUAL((test_type_t)test_val2, *test2);
	BOOST_TEST(std::log(-1.0 * test2), 1);

	test_type_t f = test_val3;
	test_type test3(f);
	BOOST_CHECK_EQUAL((test_type_t)test_val3, test3);
	BOOST_CHECK_EQUAL(test3, f);
	BOOST_CHECK_NE(test2, test3);
	BOOST_CHECK_EQUAL(false, (test2 == test3));
	BOOST_CHECK_EQUAL(true, (test2 != test3));
	BOOST_TEST((test_type_t)test3 * (test_type_t)test3, 45.891);

	test3 = test2;
	BOOST_CHECK_EQUAL((test_type_t)test_val2, test3);
	BOOST_CHECK_NE(test3, f);						// The copy breaks the original reference
	BOOST_CHECK_EQUAL(test2, test3);
	BOOST_CHECK_EQUAL(true, (test2 == test3));
	BOOST_CHECK_EQUAL(false, (test2 != test3));
	BOOST_TEST(std::log(-1.0 * test3), 1);
	BOOST_TEST(std::log(-1.0 * f), 1);

	test_type test5;

	// Try 25 random integers
	for (size_t i = 0; i < 25; i++) {
		test_type_t test_val5 = FloatGenerator::number<test_type_t>();

		test5 = test_val5;
		BOOST_CHECK_EQUAL(test_val5, test5);
	}
}

BOOST_AUTO_TEST_CASE( test_float32_type ) {
	static constexpr bool b =
			std::is_same<typename tmx::common::types::Floatmin::value_type, float>::value;
	BOOST_CHECK_EQUAL(true, b);
	BOOST_CHECK_EQUAL(32, tmx::common::types::Floatmin::numBits);
	check_float_type<float, 4, 32>();
}


BOOST_AUTO_TEST_CASE( test_float64_type ) {
	static constexpr bool b =
			std::is_same<typename tmx::common::types::Double::value_type, double>::value;
	BOOST_CHECK_EQUAL(true, b);
	BOOST_CHECK_EQUAL(64, tmx::common::types::Double::numBits);
	check_float_type<double, 8, 64>();
}

BOOST_AUTO_TEST_CASE( test_float80_type ) {
	// Note that the long double might be 10 bytes, but it might also be 16 bytes
	// The TMX_FLOAT128 is only necessary in the cases in which long double
	// is not already 16 bytes. This test is only named float80 for convenience
	// if a separate 128 byte float is also used.
#ifndef TMX_FLOAT128
	static constexpr bool b =
			std::is_same<typename tmx::common::types::Floatmax::value_type, TMX_MAX_FLOAT>::value;
	BOOST_CHECK_EQUAL(true, b);
	BOOST_CHECK_EQUAL(sizeof(TMX_MAX_FLOAT) * 8, tmx::common::types::Floatmax::numBits);
#endif
	check_float_type<long double, sizeof(long double), sizeof(long double) * 8>();
}

#ifdef TMX_FLOAT128
BOOST_AUTO_TEST_CASE( test_float128_type ) {
	static constexpr bool b =
			std::is_same<typename tmx::common::types::Floatmax::value_type, TMX_MAX_FLOAT>::value;
	BOOST_CHECK_EQUAL(true, b);
	BOOST_CHECK_EQUAL(sizeof(TMX_MAX_FLOAT) * 8, tmx::common::types::Floatmax::numBits);
	check_float_type<TMX_FLOAT128, 16, 1128>();
}
#endif

