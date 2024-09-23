/*!
 * Copyright (c) 2021 Battelle Memorial Institute
 *
 * All Rights Reserved.
 *
 * @file test_type_traits.cpp
 *
 *  Created on: Jun 23, 2021
 *      @author: Gregory M. Baumgardner
 */

#include <tmx/common/types/support/type_traits.hpp>

#include <boost/test/unit_test.hpp>
#include <type_traits>

using namespace std;
using namespace tmx::common::types::support;

struct EmptyStruct {};
struct WrappedStruct { typedef int type; };
struct SubStruct: public WrappedStruct { static constexpr const char *name = "MyName"; };

BOOST_AUTO_TEST_CASE( test_type_of ) {
	BOOST_CHECK_EQUAL( has_type<EmptyStruct>(), false);
	BOOST_CHECK_EQUAL( has_type<WrappedStruct>(), true);
	BOOST_CHECK_EQUAL( has_type<SubStruct>(), true);

	int n = 2;
	SubStruct s;
	BOOST_CHECK_EQUAL( has_type(n), false);
	BOOST_CHECK_EQUAL( has_type(s), true);

	auto check = is_same_v<type_of<EmptyStruct>, type_of<WrappedStruct>>;
	BOOST_CHECK_EQUAL( check, false );
	check = is_same_v<type_of<EmptyStruct>, type_of<EmptyStruct>>;
	BOOST_CHECK_EQUAL( check, true );
	check = is_same_v<type_of<WrappedStruct>, type_of<WrappedStruct>>;
	BOOST_CHECK_EQUAL( check, true );
	check = is_same_v<type_of<WrappedStruct>,type_of<int>>;
	BOOST_CHECK_EQUAL( check, true );
	check = is_same_v<type_of<WrappedStruct>, type_of<decltype(n)>>;
	BOOST_CHECK_EQUAL( check, true );
	check = is_same_v<type_of<WrappedStruct>, type_of<SubStruct>>;
	BOOST_CHECK_EQUAL( check, true );
}

BOOST_AUTO_TEST_CASE( test_name_of ) {
	BOOST_CHECK_EQUAL( has_name<EmptyStruct>(), false );
	BOOST_CHECK_EQUAL( has_name<WrappedStruct>(), false );
	BOOST_CHECK_EQUAL( has_name<SubStruct>(), true );

	int n = 2;
	SubStruct s;
	BOOST_CHECK_EQUAL( has_name(n), false );
	BOOST_CHECK_EQUAL( has_name(s), true );

	// BOOST_CHECK_EQUAL( name_of<EmptyStruct>(), nullptr );
	// BOOST_CHECK_EQUAL( name_of(n), nullptr );
	BOOST_CHECK_EQUAL( name_of(s), "MyName" );
}
