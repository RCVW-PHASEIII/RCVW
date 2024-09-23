/*!
 * Copyright (c) 2021 Battelle Memorial Institute
 *
 * All Rights Reserved.
 *
 * @file test_byte_string.cpp
 *
 *  Created on: Jun 17, 2021
 *      @author: Gregory M. Baumgardner
 */

#include <array>
#include <cstring>
#include <list>
#include <string>
#include <vector>
#include <iostream>

#include <boost/test/unit_test.hpp>
#include <tmx/common/types/bytes_type.hpp>

using namespace std;
using namespace tmx::common::types;
using namespace tmx::common::types::support;
using namespace tmx::common::types::enums::ostream_operators;

BOOST_AUTO_TEST_CASE( test_byte_order ) {
	static constexpr const uint32_t _tst_byte_order = 0x01ffff00;
	static auto testVal = *((const uint8_t *)&_tst_byte_order);

	// Check that all known byte orderings can be determined
	BOOST_CHECK_EQUAL(get_byte_order<0>(), TMX_LITTLE_ENDIAN);
	BOOST_CHECK_EQUAL(get_byte_order<1>(), TMX_BIG_ENDIAN);
	BOOST_CHECK_EQUAL(get_byte_order(LITTLE_ENDIAN), TMX_LITTLE_ENDIAN);
	BOOST_CHECK_EQUAL(get_byte_order(BIG_ENDIAN), TMX_BIG_ENDIAN);
#ifdef PDP_ENDIAN
	BOOST_CHECK_EQUAL(get_byte_order(PDP_ENDIAN), byte_order::unknown);
#endif
	BOOST_CHECK_EQUAL(TMX_NETWORK_BYTE_ORDER, TMX_BIG_ENDIAN);

	// Check that the default compiler-time byte ordering is correct
	auto byteOrder = get_byte_order(testVal);

	BOOST_CHECK_NE(byteOrder, byte_order::unknown);
	BOOST_CHECK_EQUAL(byteOrder, get_byte_order());
}

BOOST_AUTO_TEST_CASE( test_byte_type ) {
	const char *testStr = "First test";
	auto b1 = bytes_type<>(testStr, ::strlen(testStr));

	BOOST_CHECK_EQUAL(b1.byte_order, TMX_BIG_ENDIAN);
	BOOST_CHECK_EQUAL(b1.length(), ::strlen(testStr));

	for (size_t i = 0; i < ::strlen(testStr); i++)
		BOOST_CHECK_EQUAL((char)b1[i], testStr[i]);

	decltype(b1) b2 { b1 };
	BOOST_CHECK_EQUAL(b1.compare(b2), 0);
	BOOST_CHECK_LT(b1.compare(b2.substr(3)), 0);

	bytes_type<decltype(b1)::byte_type, TMX_LITTLE_ENDIAN> b3;
	b3 = ntoh_bytes(b1);

}

BOOST_AUTO_TEST_CASE ( test_byte_string_converters ) {

}
