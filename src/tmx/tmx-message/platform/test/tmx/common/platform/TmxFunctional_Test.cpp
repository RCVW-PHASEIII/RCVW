/*!
 * Copyright (c) 2023 Battelle Memorial Institute
 *
 * All Rights Reserved.
 *
 * @file TmxFunctional_Test.cpp
 *
 *  Created on: Aug 11, 2023
 *      @author: Gregory M. Baumgardner
 */

#include <tmx/common/platform/functional/functional.hpp>
#include <tmx/common/platform/types/introspect.hpp>

#include <boost/test/unit_test_log.hpp>
#include <boost/test/unit_test.hpp>

#include <tuple>
#include <type_traits>
#include <string>

using namespace tmx::common;

int noop(void) noexcept { return 1; }

struct MyFnTestClass {
    double div_two(double, double) const noexcept { return 0.0; }
};

BOOST_AUTO_TEST_CASE ( tmxtest_global_function ) {
    const auto fn1 = &std::make_tuple<float, unsigned char, std::string>;

    typedef decltype(fn1) fn_type;
    std::cout << type_fqname<fn_type> () << std::endl;
    std::cout << type_fqname<typename std::remove_const<fn_type>::type> () << std::endl;
    typedef function<fn_type> test1;
    BOOST_CHECK_EQUAL(test1::is_member::value, false);
    BOOST_CHECK_EQUAL(test1::is_const_member::value, false);
    BOOST_CHECK_EQUAL(test1::is_noexcept::value, false);
    BOOST_CHECK_EQUAL(test1::num_arguments, 3);
    BOOST_CHECK_EQUAL((std::is_same_v<test1::class_type, void>), true);
    BOOST_CHECK_EQUAL((std::is_same_v<test1::result_type, std::tuple<float, unsigned char, std::string> >), true);
    BOOST_CHECK_EQUAL((std::is_same_v<test1::arguments, std::tuple<float &&, unsigned char &&, std::string &&> >), true);
    BOOST_CHECK_EQUAL((std::is_same_v<test1::type, std::function<test1::result_type(float &&, unsigned char &&, std::string &&)> >), true);
    BOOST_CHECK_EQUAL((std::is_same_v<first_argument_type<fn_type>, float>), true);
    BOOST_CHECK_EQUAL((std::is_same_v<second_argument_type<fn_type>, unsigned char>), true);
    BOOST_CHECK_EQUAL((std::is_same_v<third_argument_type<fn_type>, std::string>), true);
    BOOST_CHECK_EQUAL((std::is_same_v<fourth_argument_type<fn_type>, void>), true);
    BOOST_CHECK_EQUAL((std::is_same_v<fifth_argument_type<fn_type>, void>), true);
    BOOST_CHECK_EQUAL((std::is_same_v<sixth_argument_type<fn_type>, void>), true);
    BOOST_CHECK_EQUAL((std::is_same_v<seventh_argument_type<fn_type>, void>), true);
    BOOST_CHECK_EQUAL((std::is_same_v<eighth_argument_type<fn_type>, void>), true);

    typedef function<decltype(&noop)> test2;
    std::cout << type_fqname<decltype(&noop)> () << std::endl;
    std::cout << type_fqname<typename std::remove_const<decltype(&noop)>::type> () << std::endl;
    BOOST_CHECK_EQUAL(test1::is_member::value, false);
    BOOST_CHECK_EQUAL(test1::is_const_member::value, false);
    BOOST_CHECK_EQUAL(test1::is_noexcept::value, true);
    BOOST_CHECK_EQUAL(test2::num_arguments, 0);

    BOOST_CHECK_EQUAL((std::is_same_v<test2::class_type, void>), true);
    BOOST_CHECK_EQUAL((std::is_same_v<test2::arguments, std::tuple<> >), true);
    BOOST_CHECK_EQUAL((std::is_same_v<test2::type, std::function<int()> >), true);
    BOOST_CHECK_EQUAL((std::is_same_v<result_of<test2::type>, int>), true);
}

BOOST_AUTO_TEST_CASE ( tmxtest_member_function ) {
    const auto fn1 = &MyFnTestClass::div_two;

    typedef decltype(fn1) fn_type;
    typedef function<fn_type> test1;
    BOOST_CHECK_EQUAL(test1::is_member::value, true);
    BOOST_CHECK_EQUAL(test1::is_const_member::value, true);
    BOOST_CHECK_EQUAL(test1::is_noexcept::value, true);
    BOOST_CHECK_EQUAL(test1::num_arguments, 2);
    BOOST_CHECK_EQUAL((std::is_same_v<test1::class_type, MyFnTestClass>), true);
    BOOST_CHECK_EQUAL((std::is_same_v<test1::result_type, double>), true);
    BOOST_CHECK_EQUAL((std::is_same_v<test1::arguments, std::tuple<double, double> >), true);
    BOOST_CHECK_EQUAL((std::is_same_v<test1::type, std::function<double(double, double)> >), true);
    BOOST_CHECK_EQUAL((std::is_same_v<first_argument_type<fn_type>, double>), true);
    BOOST_CHECK_EQUAL((std::is_same_v<second_argument_type<fn_type>, double>), true);
    BOOST_CHECK_EQUAL((std::is_same_v<third_argument_type<fn_type>, void>), true);
    BOOST_CHECK_EQUAL((std::is_same_v<fourth_argument_type<fn_type>, void>), true);
    BOOST_CHECK_EQUAL((std::is_same_v<fifth_argument_type<fn_type>, void>), true);
    BOOST_CHECK_EQUAL((std::is_same_v<sixth_argument_type<fn_type>, void>), true);
    BOOST_CHECK_EQUAL((std::is_same_v<seventh_argument_type<fn_type>, void>), true);
    BOOST_CHECK_EQUAL((std::is_same_v<eighth_argument_type<fn_type>, void>), true);
}

BOOST_AUTO_TEST_CASE ( tmxtest_lamba_function ) {
    const auto fn1 = [](std::string &&) { return -1; };

    typedef decltype(fn1) fn_type;
    typedef function<fn_type> test1;
    BOOST_CHECK_EQUAL(test1::is_member::value, true);
    BOOST_CHECK_EQUAL(test1::is_const_member::value, true);
    BOOST_CHECK_EQUAL(test1::is_noexcept::value, false);
    BOOST_CHECK_EQUAL(test1::num_arguments, 1);
    BOOST_CHECK_EQUAL((std::is_same_v<test1::result_type, int>), true);
    BOOST_CHECK_EQUAL((std::is_same_v<test1::arguments, std::tuple<std::string &&> >), true);
    BOOST_CHECK_EQUAL((std::is_same_v<test1::type, std::function<int(std::string &&)> >), true);
    BOOST_CHECK_EQUAL((std::is_same_v<first_argument_type<fn_type>, std::string>), true);
    BOOST_CHECK_EQUAL((std::is_same_v<third_argument_type<fn_type>, void>), true);
    BOOST_CHECK_EQUAL((std::is_same_v<fourth_argument_type<fn_type>, void>), true);
    BOOST_CHECK_EQUAL((std::is_same_v<fifth_argument_type<fn_type>, void>), true);
    BOOST_CHECK_EQUAL((std::is_same_v<sixth_argument_type<fn_type>, void>), true);
    BOOST_CHECK_EQUAL((std::is_same_v<seventh_argument_type<fn_type>, void>), true);
    BOOST_CHECK_EQUAL((std::is_same_v<eighth_argument_type<fn_type>, void>), true);

    auto fn2 = [](std::string &s) mutable { s = "Test"; };

    typedef decltype(fn2) fn2_type;
    typedef function<fn2_type> test2;
    BOOST_CHECK_EQUAL(test2::is_member::value, true);
    BOOST_CHECK_EQUAL(test2::is_const_member::value, false);
    BOOST_CHECK_EQUAL(test2::is_noexcept::value, false);
    BOOST_CHECK_EQUAL(test2::num_arguments, 1);
    BOOST_CHECK_EQUAL((std::is_same_v<test2::result_type, void>), true);
    BOOST_CHECK_EQUAL((std::is_same_v<test2::arguments, std::tuple<std::string &> >), true);
    BOOST_CHECK_EQUAL((std::is_same_v<test2::type, std::function<void(std::string &)> >), true);
    BOOST_CHECK_EQUAL((std::is_same_v<first_argument_type<fn2_type>, std::string>), true);
    BOOST_CHECK_EQUAL((std::is_same_v<third_argument_type<fn2_type>, void>), true);
    BOOST_CHECK_EQUAL((std::is_same_v<fourth_argument_type<fn2_type>, void>), true);
    BOOST_CHECK_EQUAL((std::is_same_v<fifth_argument_type<fn2_type>, void>), true);
    BOOST_CHECK_EQUAL((std::is_same_v<sixth_argument_type<fn2_type>, void>), true);
    BOOST_CHECK_EQUAL((std::is_same_v<seventh_argument_type<fn2_type>, void>), true);
    BOOST_CHECK_EQUAL((std::is_same_v<eighth_argument_type<fn2_type>, void>), true);
}

BOOST_AUTO_TEST_CASE ( tmxtest_bind_function ) {
}
