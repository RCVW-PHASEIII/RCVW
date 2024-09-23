/*!
 * Copyright (c) 2021 Battelle Memorial Institute
 *
 * All Rights Reserved.
 *
 * @file test_boolean_type.cpp
 *
 *  Created on: Jun 21, 2021
 *      @author: Gregory M. Baumgardner
 */

#include <boost/test/unit_test.hpp>

#include <bitset>
#include <iostream>

#include <tmx/common/types/Boolean.hpp>

using namespace std;
using namespace tmx::common::types;

BOOST_AUTO_TEST_CASE( tmxtest_boolean_type ) {
    boolean_type test1;
    BOOST_CHECK_EQUAL(false, test1);
    BOOST_CHECK_EQUAL(true, !test1);

   	boolean_type test2(false);
    BOOST_CHECK_EQUAL(false, *test2);

    bool b3 = true;
    Boolean test3(b3);
    BOOST_CHECK_EQUAL(true, b3);
	BOOST_CHECK_NE(test2, test3);
	BOOST_CHECK_EQUAL(test2, false);
	BOOST_CHECK_EQUAL(false, test2);
	BOOST_CHECK_EQUAL(test3, true);
	BOOST_CHECK_EQUAL(true, test3);

    test1 = true;
    test2 = true;
    b3 = false;

    BOOST_CHECK_EQUAL(true, test1);
    BOOST_CHECK_EQUAL(true, *test2);
    BOOST_CHECK_EQUAL(false, test3);
    BOOST_CHECK_EQUAL(false, b3);

    Boolean test4(test3);
    BOOST_CHECK_EQUAL(false, b3);
	BOOST_CHECK_EQUAL(test3, test4);
	BOOST_CHECK_EQUAL(test3, false);
	BOOST_CHECK_EQUAL(false, test3);
	BOOST_CHECK_EQUAL(test4, false);
	BOOST_CHECK_EQUAL(false, test4);

    b3 = true;
    BOOST_CHECK_EQUAL(true, b3);
	BOOST_CHECK_EQUAL(test3, test4);
	BOOST_CHECK_EQUAL(test3, true);
	BOOST_CHECK_EQUAL(true, test3);
	BOOST_CHECK_EQUAL(test4, true);
	BOOST_CHECK_EQUAL(true, test4);

 	BOOST_CHECK_EQUAL(TmxTypeTraits<decltype(test1)>::name, "Boolean");
}
