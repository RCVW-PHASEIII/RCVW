/*!
 * Copyright (c) 2022 Battelle Memorial Institute
 *
 * All Rights Reserved.
 *
 * @file TmxStaticArray_Test.cpp
 *
 * This file contains unit test cases on the use
 * of static arrays, including static strings.
 *
 *  Created on: Sep 28, 2022
 *      @author: Gregory M. Baumgardner
 */
#include <tmx/common/platform/types/arrays.hpp>

#include <boost/test/unit_test_log.hpp>
#include <boost/test/unit_test.hpp>

#include <iostream>
#include <type_traits>

using namespace tmx::common;

BOOST_AUTO_TEST_CASE( tmxtest_static_array ) {
	// Check generic static array of numbers
	typedef static_array<int, 20, -56, 112233, 0, -782> test_type1;

	BOOST_CHECK_EQUAL(test_type1::size, 5);

	// This is a copy
	auto array1 = test_type1::array;
	bool test1 = std::is_same_v<typename decltype(array1)::value_type, int>;
	BOOST_CHECK_EQUAL(test1, true);
	BOOST_CHECK_EQUAL(array1.size(), 5);
	BOOST_CHECK_EQUAL(array1.empty(), false);
	BOOST_CHECK_EQUAL(array1[0], 20);
	BOOST_CHECK_EQUAL(array1[1], -56);
	BOOST_CHECK_EQUAL(array1[2], 112233);
	BOOST_CHECK_EQUAL(array1[3], 0);
	BOOST_CHECK_EQUAL(array1[4], -782);

	test_type1 instance1;
	BOOST_CHECK_EQUAL(is_static_string(instance1), false);
	BOOST_CHECK_EQUAL(instance1.data(), test_type1::data());  // There should be only one static array instance
	BOOST_CHECK_NE((const int *)instance1, array1.data());  // But, the copies should be different
	BOOST_CHECK_EQUAL(instance1.at(0), 20);
	BOOST_CHECK_EQUAL(instance1.at(1), -56);
	BOOST_CHECK_EQUAL(instance1.at(2), 112233);
	BOOST_CHECK_EQUAL(instance1.at(3), 0);
	BOOST_CHECK_EQUAL(instance1.at(4), -782);

	// Try with a set of bytes
	typedef static_array<uint8_t, 0x3d, 0xff, 0x01, 0xa9, 0xd1, 0x55, 0x7c> test_type2;

	BOOST_CHECK_EQUAL(test_type2::size, 7);

	// This is a copy
	auto array2 = test_type2::array;
	bool test2 = std::is_same_v<typename decltype(array2)::value_type, uint8_t>;
	BOOST_CHECK_EQUAL(test2, true);
	BOOST_CHECK_EQUAL(array2.size(), 7);
	BOOST_CHECK_EQUAL(array2.empty(), false);
	BOOST_CHECK_EQUAL(array2[0], 0x3d);
	BOOST_CHECK_EQUAL(array2[1], 0xff);
	BOOST_CHECK_EQUAL(array2[2], 0x01);
	BOOST_CHECK_EQUAL(array2[3], 0xa9);
	BOOST_CHECK_EQUAL(array2[4], 0xd1);
	BOOST_CHECK_EQUAL(array2[5], 0x55);
	BOOST_CHECK_EQUAL(array2[6], 0x7c);

	test_type2 instance2;
	BOOST_CHECK_EQUAL(is_static_string(instance2), false);
	BOOST_CHECK_EQUAL(instance2.data(), test_type2::data());  // There should be only one static array instance
	BOOST_CHECK_NE((const uint8_t *)instance2, array2.data());  // But, the copies should be different
	BOOST_CHECK_EQUAL(instance2.at(0), 0x3d);
	BOOST_CHECK_EQUAL(instance2.at(1), 0xff);
	BOOST_CHECK_EQUAL(instance2.at(2), 0x01);
	BOOST_CHECK_EQUAL(instance2.at(3), 0xa9);
	BOOST_CHECK_EQUAL(instance2.at(4), 0xd1);
	BOOST_CHECK_EQUAL(instance2.at(5), 0x55);
	BOOST_CHECK_EQUAL(instance2.at(6), 0x7c);

	array1.fill('\0');

	// Make a countable array
	static constexpr auto instance3 = make_array(std::make_integer_sequence<uint16_t, 50> {});
	typedef decltype(instance3) test_type3;

	BOOST_CHECK_EQUAL(test_type3::size, 50);

	// This is a copy
	auto array3 = test_type3::array;

	bool test3 = std::is_same_v<typename decltype(array3)::value_type, uint16_t>;
	BOOST_CHECK_EQUAL(test3, true);
	BOOST_CHECK_EQUAL(array3.size(), 50);
	BOOST_CHECK_EQUAL(array3.empty(), false);
	for (std::size_t i = 0; i < 50; i++)
		BOOST_CHECK_EQUAL(array3[i], i);

	BOOST_CHECK_EQUAL(is_static_string(instance3), false);
	BOOST_CHECK_EQUAL(instance3.data(), test_type3::data());  // There should be only one static array instance
	BOOST_CHECK_NE((const uint16_t *)instance3, array3.data());  // But, the copies should be different
	for (std::size_t i = 0; i < 50; i++)
		BOOST_CHECK_EQUAL(instance3.at(i), i);

	array1.fill('\0');
}

BOOST_AUTO_TEST_CASE( tmxtest_static_string ) {
	typedef static_array<char, 'I', ' ', 'h', 'e', 'a', 'r', 't', ' ', 'T', 'M', 'X'> test_type1;

	BOOST_CHECK_EQUAL(test_type1::size, 11);

	// This is a copy
	auto array1 = test_type1::array;
	bool test1 = std::is_same_v<typename decltype(array1)::value_type, char>;
	BOOST_CHECK_EQUAL(test1, true);
	BOOST_CHECK_EQUAL(array1.size(), 11);
	BOOST_CHECK_EQUAL(array1.empty(), false);
	BOOST_CHECK_EQUAL(array1[0], 'I');
	BOOST_CHECK_EQUAL(array1[1], ' ');
	BOOST_CHECK_EQUAL(array1[2], 'h');
	BOOST_CHECK_EQUAL(array1[3], 'e');
	BOOST_CHECK_EQUAL(array1[4], 'a');
	BOOST_CHECK_EQUAL(array1[5], 'r');
	BOOST_CHECK_EQUAL(array1[6], 't');
	BOOST_CHECK_EQUAL(array1[7], ' ');
	BOOST_CHECK_EQUAL(array1[8], 'T');
	BOOST_CHECK_EQUAL(array1[9], 'M');
	BOOST_CHECK_EQUAL(array1[10], 'X');

	test_type1 instance1;
	BOOST_CHECK_EQUAL(is_static_string(instance1), true);
	BOOST_CHECK_EQUAL(instance1.c_str(), test_type1::c_str());  // There should be only one static character array instance
	BOOST_CHECK_NE((std::uintptr_t)(const char *)instance1, (std::uintptr_t)array1.data());  // But, the copies should be different
	BOOST_CHECK_EQUAL(instance1.at(0), 'I');
	BOOST_CHECK_EQUAL(instance1.at(1), ' ');
	BOOST_CHECK_EQUAL(instance1.at(2), 'h');
	BOOST_CHECK_EQUAL(instance1.at(3), 'e');
	BOOST_CHECK_EQUAL(instance1.at(4), 'a');
	BOOST_CHECK_EQUAL(instance1.at(5), 'r');
	BOOST_CHECK_EQUAL(instance1.at(6), 't');
	BOOST_CHECK_EQUAL(instance1.at(7), ' ');
	BOOST_CHECK_EQUAL(instance1.at(8), 'T');
	BOOST_CHECK_EQUAL(instance1.at(9), 'M');
	BOOST_CHECK_EQUAL(instance1.at(10), 'X');

	std::string teststr { instance1 };
	BOOST_CHECK_EQUAL(teststr, "I heart TMX");

	array1.fill('\0');

	// Convert from constant C-style string
	static constexpr auto testString = "https://www.example.com";
	auto instance2 = make_string<&(testString)>();
	typedef decltype(instance2) test_type2;

	BOOST_CHECK_EQUAL(test_type2::size, 23);

	// This is a copy
	auto array2 = test_type2::array;
	bool test2 = std::is_same_v<typename decltype(array2)::value_type, char>;
	BOOST_CHECK_EQUAL(test2, true);
	BOOST_CHECK_EQUAL(array2.size(), 23);
	BOOST_CHECK_EQUAL(array2.empty(), false);
	for (std::size_t i = 0; i < 23; i++)
		BOOST_CHECK_EQUAL(array2[i], testString[i]);

	BOOST_CHECK_EQUAL(is_static_string(instance2), true);
	BOOST_CHECK_EQUAL(instance2.c_str(), test_type2::c_str());  // There should be only one static character array instance
	BOOST_CHECK_EQUAL((const char *)instance2, array2.data());  // Even the copies should be from the same source
	for (std::size_t i = 0; i < 23; i++)
		BOOST_CHECK_EQUAL(instance2.at(i), testString[i]);

	teststr.assign(instance2);
	BOOST_CHECK_EQUAL(teststr, testString);

	array2.fill('\0');
}

BOOST_AUTO_TEST_CASE( tmxtest_static_array_literals ) {
	static constexpr auto test1 = 459_tmx_a;
	typedef decltype(test1) test_type1;
    typedef typename std::decay<decltype(test_type1::array)>::type array_type;
	bool chk1 = std::is_same_v<typename array_type::value_type, uint8_t>;

	BOOST_CHECK_EQUAL(test_type1::size, 3);
	BOOST_CHECK_EQUAL(chk1, true);
	BOOST_CHECK_EQUAL(test1.at(0), 4);
	BOOST_CHECK_EQUAL(test1.at(1), 5);
	BOOST_CHECK_EQUAL(test1.at(2), 9);

	static constexpr auto test2 = "Custom string literal"_tmx_s;
	typedef decltype(test2) test_type2;
	bool chk2 = std::is_same_v<typename decltype(test_type2::array)::value_type, char>;

	BOOST_CHECK_EQUAL(test_type2::size, 21);
	BOOST_CHECK_EQUAL(chk2, true);
	BOOST_CHECK_EQUAL(test2.at(0), 'C');
	BOOST_CHECK_EQUAL(test2.at(1), 'u');
	BOOST_CHECK_EQUAL(test2.at(2), 's');
	BOOST_CHECK_EQUAL(test2.at(3), 't');
	BOOST_CHECK_EQUAL(test2.at(4), 'o');
	BOOST_CHECK_EQUAL(test2.at(5), 'm');
	BOOST_CHECK_EQUAL(test2.at(6), ' ');
	BOOST_CHECK_EQUAL(test2.at(7), 's');
	BOOST_CHECK_EQUAL(test2.at(8), 't');
	BOOST_CHECK_EQUAL(test2.at(9), 'r');
	BOOST_CHECK_EQUAL(test2.at(10), 'i');
	BOOST_CHECK_EQUAL(test2.at(11), 'n');
	BOOST_CHECK_EQUAL(test2.at(12), 'g');
	BOOST_CHECK_EQUAL(test2.at(13), ' ');
	BOOST_CHECK_EQUAL(test2.at(14), 'l');
	BOOST_CHECK_EQUAL(test2.at(15), 'i');
	BOOST_CHECK_EQUAL(test2.at(16), 't');
	BOOST_CHECK_EQUAL(test2.at(17), 'e');
	BOOST_CHECK_EQUAL(test2.at(18), 'r');
	BOOST_CHECK_EQUAL(test2.at(19), 'a');
	BOOST_CHECK_EQUAL(test2.at(20), 'l');

	std::string teststr { test2.c_str() };
	BOOST_CHECK_EQUAL(teststr, "Custom string literal");

	static constexpr auto test3 = 123456_tmx_s;
	typedef decltype(test3) test_type3;
	bool chk3 = std::is_same_v<typename decltype(test_type2::array)::value_type, char>;

	BOOST_CHECK_EQUAL(test_type3::size, 6);
	BOOST_CHECK_EQUAL(chk3, true);
	BOOST_CHECK_EQUAL(test3.at(0), '1');
	BOOST_CHECK_EQUAL(test3.at(1), '2');
	BOOST_CHECK_EQUAL(test3.at(2), '3');
	BOOST_CHECK_EQUAL(test3.at(3), '4');
	BOOST_CHECK_EQUAL(test3.at(4), '5');
	BOOST_CHECK_EQUAL(test3.at(5), '6');

	teststr.assign(test3);
	BOOST_CHECK_EQUAL(teststr, "123456");
}

BOOST_AUTO_TEST_CASE( tmxtest_static_array_concat ) {
	static constexpr char str[] = "The first one";
	static constexpr auto s = const_cast<const char *>(str);
	static constexpr auto one = TMX_STATIC_STRING_VAR(s);
	static constexpr auto two = TMX_STATIC_STRING("the next");

	static constexpr auto oneNTwo = concat(concat(one, TMX_STATIC_STRING(" and ")), two);

	bool test1 = std::is_same_v<std::decay_t<decltype(oneNTwo)>,
			static_array<char, 'T', 'h', 'e', ' ', 'f', 'i', 'r', 's', 't',
				' ', 'o', 'n', 'e', ' ', 'a', 'n', 'd', ' ', 't', 'h', 'e',
				' ', 'n', 'e', 'x', 't'> >;
	BOOST_CHECK_EQUAL(test1, true);

	std::string teststr { oneNTwo };
	BOOST_CHECK_EQUAL(teststr, "The first one and the next");
}

BOOST_AUTO_TEST_CASE( tmxtest_static_array_digitize ) {
	static constexpr auto test1 = "2d9FC104"_tmx_a; // Intentionally using lower case and upper case
	typedef decltype(test1) test_type1;

	BOOST_CHECK_EQUAL(test_type1::size, 8);
	BOOST_CHECK_EQUAL(test1.at(0), 2);
	BOOST_CHECK_EQUAL(test1.at(1), 13);
	BOOST_CHECK_EQUAL(test1.at(2), 9);
	BOOST_CHECK_EQUAL(test1.at(3), 15);
	BOOST_CHECK_EQUAL(test1.at(4), 12);
	BOOST_CHECK_EQUAL(test1.at(5), 1);
	BOOST_CHECK_EQUAL(test1.at(6), 0);
	BOOST_CHECK_EQUAL(test1.at(7), 4);

	static constexpr auto val1 = dedigitize<16>(test1);

	BOOST_CHECK_EQUAL(val1, 765444356ul);

	static constexpr auto test2 = "1000110001110"_tmx_a;
	typedef decltype(test2) test_type2;

	BOOST_CHECK_EQUAL(test_type2::size, 13);

	BOOST_CHECK_EQUAL(test2.at(0), 1);
	BOOST_CHECK_EQUAL(test2.at(1), 0);
	BOOST_CHECK_EQUAL(test2.at(2), 0);
	BOOST_CHECK_EQUAL(test2.at(3), 0);
	BOOST_CHECK_EQUAL(test2.at(4), 1);
	BOOST_CHECK_EQUAL(test2.at(5), 1);
	BOOST_CHECK_EQUAL(test2.at(6), 0);
	BOOST_CHECK_EQUAL(test2.at(7), 0);
	BOOST_CHECK_EQUAL(test2.at(8), 0);
	BOOST_CHECK_EQUAL(test2.at(9), 1);
	BOOST_CHECK_EQUAL(test2.at(10), 1);
	BOOST_CHECK_EQUAL(test2.at(11), 1);
	BOOST_CHECK_EQUAL(test2.at(12), 0);

	static constexpr auto val2 = dedigitize<2>(test2);

	BOOST_CHECK_EQUAL(val2, 4494ul);

	static constexpr auto test3 = static_array<unsigned int, 4, 1, 6> {};
	typedef decltype(test3) test_type3;

	BOOST_CHECK_EQUAL(test_type3::size, 3);

	BOOST_CHECK_EQUAL(test3.at(0), 4u);
	BOOST_CHECK_EQUAL(test3.at(1), 1u);
	BOOST_CHECK_EQUAL(test3.at(2), 6u);

	static constexpr auto val3 = dedigitize(test3);
	static constexpr auto val4 = dedigitize<8>(test3);

	BOOST_CHECK_EQUAL(val3, 416ul);
	BOOST_CHECK_EQUAL(val4, 270ul);
}

