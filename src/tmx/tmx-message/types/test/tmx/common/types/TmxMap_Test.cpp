/*!
 * Copyright (c) 2021 Battelle Memorial Institute
 *
 * All Rights Reserved.
 *
 * @file TmxMap_Test.cpp
 *
 *  Created on: Jun 21, 2021
 *      @author: Alex Schonbaum
 */

#include <tmx/common/types/Map.hpp>

#include <boost/test/unit_test.hpp>

#include <bitset>
#include <iostream>
#include <array>

using namespace std;
using namespace tmx::common::types;

BOOST_AUTO_TEST_CASE( test_map_type ) {

    map_type<int,int> m1;

    /*
    BOOST_CHECK_EQUAL(m1.empty(), true);
    BOOST_CHECK_EQUAL(m1.empty(), true);

    map_type<int,int> m2 {{1,2},{3,4},{5,6}};

    std::map<int,int> m3 {{1,2},{3,4},{5,6}};

    m1 = m3;

    BOOST_CHECK_EQUAL(m1.empty(), false);
    BOOST_CHECK_EQUAL(m1.size(), 3);

    map_type<int,int> m4(m3);
    map_type<int,int> m5({{1,2},{3,4},{5,6}});

    BOOST_CHECK_EQUAL(m1[1], 2);
    BOOST_CHECK_EQUAL(m2[5], 6);
    BOOST_CHECK_EQUAL(m2.at(5), 6);

    int asdf = m5.count(1);
    BOOST_CHECK_EQUAL(asdf, 1);
 

    BOOST_CHECK_EQUAL(m5.find(3)->second,4);

    BOOST_CHECK_EQUAL(m5.begin()->second,2);
    BOOST_CHECK_EQUAL(m5.cbegin()->second,2);


    BOOST_CHECK_EQUAL(m5.end()->second,0);
    BOOST_CHECK_EQUAL(m5.cend()->second,0);


    m5.emplace(7,8);
    BOOST_CHECK_EQUAL(m5.at(7),8);


    BOOST_CHECK_EQUAL(m5.max_size(),m3.max_size());
	*/

}
