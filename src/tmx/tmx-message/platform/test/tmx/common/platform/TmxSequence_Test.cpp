/*
 * Copyright (c) 2022 Battelle Memorial Institute
 *
 * All Rights Reserved.
 *
 * @file TmxSequence_Test.cpp
 *
 *  Created on: Oct 1, 2022
 *      @author: Gregory M. Baumgardner
 */

#include <tmx/common/platform/iteration/sequence.hpp>
#include <tmx/common/platform/types/arrays.hpp>
#include <tmx/common/platform/types/introspect.hpp>

#include <boost/test/unit_test.hpp>
#include <boost/test/unit_test_log.hpp>

#include <array>
#include <iostream>
#include <type_traits>
#include <utility>
#include <variant>

using namespace tmx::common;

BOOST_AUTO_TEST_CASE( tmxtest_leftshift ) {
	static constexpr auto first30 = std::make_index_sequence<30> {};
	static constexpr auto test1 = leftshift_sequence<15>(first30);

	BOOST_CHECK_EQUAL(first30.size(), test1.size());

	// Need to convert to a static array to pull the values
	static constexpr auto array1 = make_array(test1);
	BOOST_CHECK_EQUAL(array1.at(0), 15);
	BOOST_CHECK_EQUAL(array1.at(1), 16);
	BOOST_CHECK_EQUAL(array1.at(2), 17);
	BOOST_CHECK_EQUAL(array1.at(3), 18);
	BOOST_CHECK_EQUAL(array1.at(4), 19);
	BOOST_CHECK_EQUAL(array1.at(5), 20);
	BOOST_CHECK_EQUAL(array1.at(6), 21);
	BOOST_CHECK_EQUAL(array1.at(7), 22);
	BOOST_CHECK_EQUAL(array1.at(8), 23);
	BOOST_CHECK_EQUAL(array1.at(9), 24);
	BOOST_CHECK_EQUAL(array1.at(10), 25);
	BOOST_CHECK_EQUAL(array1.at(11), 26);
	BOOST_CHECK_EQUAL(array1.at(12), 27);
	BOOST_CHECK_EQUAL(array1.at(14), 29);
	BOOST_CHECK_EQUAL(array1.at(15), 30);
	BOOST_CHECK_EQUAL(array1.at(16), 31);
	BOOST_CHECK_EQUAL(array1.at(17), 32);
	BOOST_CHECK_EQUAL(array1.at(18), 33);
	BOOST_CHECK_EQUAL(array1.at(19), 34);
	BOOST_CHECK_EQUAL(array1.at(20), 35);
	BOOST_CHECK_EQUAL(array1.at(21), 36);
	BOOST_CHECK_EQUAL(array1.at(22), 37);
	BOOST_CHECK_EQUAL(array1.at(23), 38);
	BOOST_CHECK_EQUAL(array1.at(24), 39);
	BOOST_CHECK_EQUAL(array1.at(25), 40);
	BOOST_CHECK_EQUAL(array1.at(26), 41);
	BOOST_CHECK_EQUAL(array1.at(27), 42);
	BOOST_CHECK_EQUAL(array1.at(28), 43);
	BOOST_CHECK_EQUAL(array1.at(29), 44);
}

BOOST_AUTO_TEST_CASE( tmxtest_rightshift ) {
	static constexpr auto first8 = std::make_integer_sequence<short, 8> {};
	static constexpr auto test1 = rightshift_sequence<90>(first8);

	BOOST_CHECK_EQUAL(first8.size(), test1.size());

	// Need to convert to a static array to pull the values
	static constexpr auto array1 = make_array(test1);
	BOOST_CHECK_EQUAL(array1.at(0), -90);
	BOOST_CHECK_EQUAL(array1.at(1), -89);
	BOOST_CHECK_EQUAL(array1.at(2), -88);
	BOOST_CHECK_EQUAL(array1.at(3), -87);
	BOOST_CHECK_EQUAL(array1.at(4), -86);
	BOOST_CHECK_EQUAL(array1.at(5), -85);
	BOOST_CHECK_EQUAL(array1.at(6), -84);
	BOOST_CHECK_EQUAL(array1.at(7), -83);
}

BOOST_AUTO_TEST_CASE( tmxtest_doubleshift ) {
	static constexpr auto first4 = std::make_index_sequence<4> {};
	static constexpr auto test1 = rightshift_sequence<14>(leftshift_sequence<15>(first4));

	BOOST_CHECK_EQUAL(first4.size(), test1.size());

	// Need to convert to a static array to pull the values
	static constexpr auto array1 = make_array(test1);
	BOOST_CHECK_EQUAL(array1.at(0), 1);
	BOOST_CHECK_EQUAL(array1.at(1), 2);
	BOOST_CHECK_EQUAL(array1.at(2), 3);
	BOOST_CHECK_EQUAL(array1.at(3), 4);

	static constexpr auto test2 = leftshift_sequence<11>(rightshift_sequence<5>(test1));

	BOOST_CHECK_EQUAL(first4.size(), test2.size());

	// Need to convert to a static array to pull the values
	static constexpr auto array2 = make_array(test2);
	BOOST_CHECK_EQUAL(array2.at(0), 7);
	BOOST_CHECK_EQUAL(array2.at(1), 8);
	BOOST_CHECK_EQUAL(array2.at(2), 9);
	BOOST_CHECK_EQUAL(array2.at(3), 10);

	static constexpr auto test3 = leftshift_sequence<10>(rightshift_sequence<25>(test2));

	BOOST_CHECK_EQUAL(first4.size(), test3.size());

	// Need to convert to a static array to pull the values
	static constexpr auto array3 = make_array(test3);
	BOOST_CHECK_EQUAL(array3.at(0), (std::size_t)-8);
	BOOST_CHECK_EQUAL(array3.at(1), (std::size_t)-7);
	BOOST_CHECK_EQUAL(array3.at(2), (std::size_t)-6);
	BOOST_CHECK_EQUAL(array3.at(3), (std::size_t)-5);
}

BOOST_AUTO_TEST_CASE( tmxtest_make_index_sequence ) {
	static constexpr auto test1 = make_index_sequence<6, 200>();

	BOOST_CHECK_EQUAL(6, test1.size());

	// Need to convert to a static array to pull the values
	static constexpr auto array1 = make_array(test1);
	BOOST_CHECK_EQUAL(array1.at(0), 200);
	BOOST_CHECK_EQUAL(array1.at(1), 201);
	BOOST_CHECK_EQUAL(array1.at(2), 202);
	BOOST_CHECK_EQUAL(array1.at(3), 203);
	BOOST_CHECK_EQUAL(array1.at(4), 204);
	BOOST_CHECK_EQUAL(array1.at(5), 205);
}
