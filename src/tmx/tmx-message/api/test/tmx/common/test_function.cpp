/*!
 * Copyright (c) 2021 Battelle Memorial Institute
 *
 * All Rights Reserved.
 *
 * @file test_fn.cpp
 *
 *  Created on: Jul 12, 2021
 *      @author: gmb
 */

#include <tmx/common/types/support/function.hpp>

#include <boost/test/unit_test.hpp>

#include <iostream>
#include <string>

using namespace std;
using namespace tmx::common::types;
using namespace tmx::common::types::support;

BOOST_AUTO_TEST_CASE( test_addition ) {
	addition_fn<11> fn1;

	BOOST_CHECK_EQUAL(fn1(40), 51);

	typename decltype(fn1)::inverter_fn fn2;

	auto fqname = type_id_name<subtraction_fn<11>>();
	BOOST_CHECK_EQUAL(fqname, type_id_name(fn2));

	BOOST_CHECK_EQUAL(fn2(86), 75);
}

BOOST_AUTO_TEST_CASE( test_substraction ) {
	subtraction_fn<9825> fn1;

	BOOST_CHECK_EQUAL(fn1(7267), -2558);

	typename decltype(fn1)::inverter_fn fn2;
	auto fqname = type_id_name<addition_fn<9825>>();
	BOOST_CHECK_EQUAL(fqname, type_id_name(fn2));

	BOOST_CHECK_EQUAL(fn2(420), 10245);
}

BOOST_AUTO_TEST_CASE( test_multiplication ) {
	multiplication_fn<16> fn1;

	BOOST_CHECK_EQUAL(fn1(23.87), 381.92);

	typename decltype(fn1)::inverter_fn fn2;
	auto fqname = type_id_name<division_fn<16>>();
	BOOST_CHECK_EQUAL(fqname, type_id_name(fn2));

	BOOST_CHECK_EQUAL(fn2(904.0), 56.5);
}

BOOST_AUTO_TEST_CASE( test_division ) {
	division_fn<8> fn1;

	BOOST_CHECK_EQUAL(fn1(-1.0), -0.125);

	typename decltype(fn1)::inverter_fn fn2;
	auto fqname = type_id_name<multiplication_fn<8>>();
	BOOST_CHECK_EQUAL(fqname, type_id_name(fn2));

	BOOST_CHECK_EQUAL(fn2(7034), 56272);
}

BOOST_AUTO_TEST_CASE( test_power ) {
	power_fn<> fn1;

	BOOST_CHECK_EQUAL(fn1(5), 100000);

	typename decltype(fn1)::inverter_fn fn2;
	auto fqname = type_id_name<logarithm_fn<10>>();
	BOOST_CHECK_EQUAL(fqname, type_id_name(fn2));

	BOOST_CHECK_EQUAL(fn2(0.0000001), -7);

}

BOOST_AUTO_TEST_CASE( test_logarithm ) {
	logarithm_fn<2> fn1;

	BOOST_CHECK_EQUAL(fn1(1048576), 20);

	typename decltype(fn1)::inverter_fn fn2;
	auto fqname = type_id_name<power_fn<2>>();
	BOOST_CHECK_EQUAL(fqname, type_id_name(fn2));

	BOOST_CHECK_EQUAL(fn2(7), 128);
}

