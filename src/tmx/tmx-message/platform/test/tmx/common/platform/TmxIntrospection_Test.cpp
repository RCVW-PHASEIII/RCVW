/*!
 * Copyright (c) 2021 Battelle Memorial Institute
 *
 * All Rights Reserved.
 *
 * @file test_introspection.cpp
 *
 *  Created on: Jun 14, 2021
 *      @author: Gregory M. Baumgardner
 */

#include <tmx/common/platform/types/introspect.hpp>

#include <boost/test/unit_test.hpp>
#include <boost/test/unit_test_log.hpp>

#include <string>

using namespace tmx::common;

namespace tmx {
namespace test {
namespace introspect {

struct GenericStruct { };
struct WrappedStruct { typedef int type; };

template <typename T>
void test_function(T data, std::string_view str) {

}

}
}
}

struct NoNamespace { typedef int type; };

#define DECLTYPE(NM, VAR) typedef NM VAR; \
	const_string VAR##_nm { #NM }; \
	const_string VAR##_str { #NM }
#define DECLTYPENS(NS, NM, VAR) typedef NS::NM VAR; \
	const_string VAR##_ns { #NS }; \
	const_string VAR##_nm { #NM }; \
	std::string VAR##_str { std::string { #NS } + "::" + #NM }

DECLTYPENS(tmx::test::introspect, GenericStruct, test_type1);
DECLTYPE(NoNamespace, test_type2);

BOOST_AUTO_TEST_CASE( tmxtest_introspection ) {
	// Test with the type declaration
	static constexpr auto i1 = introspect<test_type1>();
	BOOST_CHECK_EQUAL(i1.name, test_type1_str);

	static constexpr auto i2 = introspect<test_type2>();
	BOOST_CHECK_EQUAL(i2.name, test_type2_str);

	// Test with references
	test_type1 test1;
	test_type2 test2;

	static constexpr auto i3 = introspect(test1);
	BOOST_CHECK_EQUAL(i3.name, test_type1_str);

	static constexpr auto i4 = introspect(test2);
	BOOST_CHECK_EQUAL(i4.name, test_type2_str);

}

BOOST_AUTO_TEST_CASE( tmxtest_typename ) {
	// Test with the type declaration
	BOOST_CHECK_EQUAL( type_fqname<test_type1>(), test_type1_str );
	BOOST_CHECK_EQUAL( type_short_name<test_type1>(), test_type1_nm );
	BOOST_CHECK_EQUAL( type_namespace<test_type1>(), test_type1_ns );

	BOOST_CHECK_EQUAL( type_fqname<test_type2>(), test_type2_str );
	BOOST_CHECK_EQUAL( type_short_name<test_type2>(), test_type2_nm );
	BOOST_CHECK_EQUAL( type_namespace<test_type2>(), "" );

	// Test with references
	test_type1 test1;
	test_type2 test2;

	BOOST_CHECK_EQUAL( type_fqname(test1), test_type1_str );
	BOOST_CHECK_EQUAL( type_short_name(test1), test_type1_nm );
	BOOST_CHECK_EQUAL( type_namespace(test1), test_type1_ns );

	BOOST_CHECK_EQUAL( type_fqname(test2), test_type2_str );
	BOOST_CHECK_EQUAL( type_short_name(test2), test_type2_nm );
	BOOST_CHECK_EQUAL( type_namespace(test2), "" );


}
