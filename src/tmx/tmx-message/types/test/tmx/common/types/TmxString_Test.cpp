/*!
 * Copyright (c) 2021 Battelle Memorial Institute
 *
 * All Rights Reserved.
 *
 * @file TmxString_Test.cpp
 *
 *  Created on: Jun 21, 2021
 *      @author: Alex Schonbaum
 */

#include <tmx/common/types/String.hpp>

#include <boost/test/unit_test.hpp>

using namespace std;
using namespace tmx::common::types;

BOOST_AUTO_TEST_CASE( test_string_type ) {
    String8 test1;
    BOOST_CHECK_EQUAL(true, test1.empty());
    BOOST_CHECK_EQUAL("", test1);

    String8 test2("test");
    BOOST_CHECK_EQUAL(false, test2.empty());
    BOOST_CHECK_EQUAL(*test2, "test");

    String8 test3({'M','y','N','a','m','e'});
    BOOST_CHECK_EQUAL(false, test2.empty());
    BOOST_CHECK_EQUAL("MyName", test3);

}
