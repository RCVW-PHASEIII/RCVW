/*!
 * Copyright (c) 2021 Battelle Memorial Institute
 *
 * All Rights Reserved.
 *
 * @file TmxArray_Test.cpp
 *
 *  Created on: Jun 21, 2021
 *      @author: Alex Schonbaum
 */

#include <tmx/common/types/Array.hpp>

#include <boost/test/unit_test.hpp>

#include <bitset>
#include <iostream>
#include <array>

using namespace std;
using namespace tmx::common::types;

BOOST_AUTO_TEST_CASE( test_array_type ) {

    array_type<int> array1;
/*    array_type<int> array2({1,2,3,4});
    const array_type<int> array3 = {1,2,3,4};
    array_type<int> array4(array3);

    std::vector<int> a1 {1,2,3,4};
    std::array<int,4> a2 {1,2,3,4};

    array_type<int> array5 = a1;
    array_type<int> array6(a1);

    // array_type<int> array7 = a2;

    BOOST_CHECK_EQUAL(array1.empty(),1);
    BOOST_CHECK_EQUAL(array2.empty(),0);

    BOOST_CHECK_EQUAL(array2.at(3),4);
    BOOST_CHECK_EQUAL(array2[3],4);

    BOOST_CHECK_EQUAL(array2.front(),1);
    BOOST_CHECK_EQUAL(array2.back(),4);
    BOOST_CHECK_EQUAL(array2.size(),4);

    BOOST_CHECK_EQUAL(array3.size(),array4.size());
    BOOST_CHECK_EQUAL(array3.front(),array4.front());
    BOOST_CHECK_EQUAL(array3.back(),array4.back());

    BOOST_CHECK_EQUAL(array6.size(), a1.size());
    BOOST_CHECK_EQUAL(array6.front(), a1.front());
    BOOST_CHECK_EQUAL(array6.back(), a1.back());
    */
}
