/*
 * Copyright (c) 2022 Battelle Memorial Institute
 *
 * All Rights Reserved.
 *
 * @file TmxEnum_Test.cpp
 *
 *  Created on: Oct 11, 2022
 *      @author: Gregory M. Baumgardner
 */

#include <iostream>
#include <tmx/common/platform/endianness/byte_order.hpp>

#include <boost/test/unit_test.hpp>

using namespace tmx::common;

BOOST_AUTO_TEST_CASE( tmxtest_byte_order ) {
	// Check that all known byte orderings can be determined
	auto test_type1 = get_byte_order<TMX_LITTLE_ENDIAN>();
	auto test_type2 = get_byte_order<TMX_BIG_ENDIAN>();
	auto test_type3 = get_byte_order<TMX_PDP_ENDIAN>();

	bool test1 = std::is_same<decltype(test_type1), little_endian>::value;
	bool test2 = std::is_same<decltype(test_type2), big_endian>::value;
	bool test3 = std::is_same<network_byte_order, big_endian>::value;
	bool test4 = std::is_same<decltype(test_type3), unknown_byte_order>::value;

	BOOST_CHECK_EQUAL(true, test1);
	BOOST_CHECK_EQUAL(true, test2);
	BOOST_CHECK_EQUAL(true, test3);
	BOOST_CHECK_EQUAL(true, test3);
	BOOST_CHECK_EQUAL(get_endianness<TMX_LITTLE_ENDIAN>(), "little_endian");
	BOOST_CHECK_EQUAL(get_endianness<TMX_BIG_ENDIAN>(), "big_endian");
	BOOST_CHECK_EQUAL(get_endianness<TMX_NETWORK_BYTE_ORDER>(), "big_endian");
	BOOST_CHECK_EQUAL(get_endianness<TMX_PDP_ENDIAN>(), "unknown_byte_order");

	// Check the byte order of the current system
	static constexpr const uint32_t _tst_byte_order = 0x01ffff00;
	static auto testVal = *((const uint8_t *)&_tst_byte_order);

	auto test_type5 = get_byte_order();
	if (testVal) {
		bool test5 = std::is_same<decltype(test_type5), big_endian>::value;
		BOOST_CHECK_EQUAL(true, test5);
		BOOST_CHECK_EQUAL(get_endianness(), "big_endian");
		BOOST_CHECK_EQUAL(true, is_big_endian());
		BOOST_CHECK_EQUAL(false, is_little_endian());
		BOOST_CHECK_EQUAL(true, is_network_byte_order());
		BOOST_CHECK_EQUAL(false, is_byte_order<TMX_PDP_ENDIAN>());
	} else {
		bool test5 = std::is_same<decltype(test_type5), little_endian>::value;
		BOOST_CHECK_EQUAL(true, test5);
		BOOST_CHECK_EQUAL(get_endianness(), "little_endian");
		BOOST_CHECK_EQUAL(false, is_big_endian());
		BOOST_CHECK_EQUAL(true, is_little_endian());
		BOOST_CHECK_EQUAL(false, is_network_byte_order());
		BOOST_CHECK_EQUAL(false, is_byte_order<TMX_PDP_ENDIAN>());
	}

}
