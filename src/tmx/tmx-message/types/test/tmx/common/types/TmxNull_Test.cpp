/*!
 * Copyright (c) 2021 Battelle Memorial Institute
 *
 * All Rights Reserved.
 *
 * @file TmxNull_Test.cpp
 *
 *  Created on: Jun 21, 2021
 *      @author: Gregory M. Baumgardner
 */

#include <tmx/common/types/Null.hpp>

#include <boost/test/unit_test.hpp>

#include <bitset>
#include <iostream>

using namespace std;
using namespace tmx::common::types;

struct emptyStruct { };

std::ostream &operator<<(std::ostream &os, emptyStruct const &) {
	return os;
}

BOOST_AUTO_TEST_CASE( tmxtest_null_string ) {
	Null test1;
	BOOST_CHECK_EQUAL(TmxTypeTraits<decltype(test1)>::name, "Null");
}
