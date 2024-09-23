/*!
 * Copyright (c) 2021 Battelle Memorial Institute
 *
 * All Rights Reserved.
 *
 * @file test_exception.cpp
 *
 *  Created on: Jun 16, 2021
 *      @author: Gregory M. Baumgardner
 */

#include <tmx/common/TmxException.hpp>
#include <boost/test/unit_test.hpp>

#include <iostream>

using namespace std;
using namespace boost::unit_test;
using namespace tmx::common;

void functionA(int num, const char *data) {
	//TmxException<> ex(data);
	//cout << ex << endl;

	//TmxException<> ex2(ex);
	//cout << ex << endl;
};

void functionB(int num, const char *data) {
	functionA(num, data);
}

BOOST_AUTO_TEST_CASE ( test_constructors ) {
	functionB(1, "Hello");

	//stacktrace bt;
//	cout << bt << endl;
}


