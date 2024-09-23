/*!
 * Copyright (c) 2021 Battelle Memorial Institute
 *
 * All Rights Reserved.
 *
 * @file test-TmxData.cpp
 *
 *  Created on: Jul 20, 2021
 *      @author: gmb
 */

#include <tmx/common/TmxData.hpp>

#include <boost/test/unit_test.hpp>

using namespace std;
using namespace tmx::common;

BOOST_AUTO_TEST_CASE( test_assignments ) {
	TmxData data;
	cout << data.contents() << endl;
	//BOOST_CHECK_EQUAL(data.contents(), "null<0>");

	data = true;
	cout << data.contents() << endl;
	//BOOST_CHECK_EQUAL(data.contents(), "boolean<1>");

	data = 15;
	cout << data.contents() << endl;
	//BOOST_CHECK_EQUAL(data.contents(), "int<32>");

	data = 3.1415;
	cout << data.contents() << endl;
	//BOOST_CHECK_EQUAL(data.contents(), "float<64>");

	data = "This is a string";
	cout << data.contents() << endl;
	//BOOST_CHECK_EQUAL(data.contents(), "string<8>");

	data = std::vector<short>({ 43, 810, -124, 1393});
	cout << data.contents() << endl;
}
