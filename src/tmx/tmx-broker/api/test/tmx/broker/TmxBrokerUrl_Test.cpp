/*!
 * Copyright (c) 2021 Battelle Memorial Institute
 *
 * All Rights Reserved.
 *
 * @file TmxClient_test.cpp
 *
 *  Created on: Aug 11, 2021
 *      @author: gmb
 */


#include <tmx/broker/TmxBrokerContext.hpp>

#include <boost/test/unit_test.hpp>

using namespace tmx::broker;

namespace tmx {
namespace broker {

BOOST_AUTO_TEST_CASE ( test_url_parse ) {
	// Check fully expanded case
	const char *testA = "explicit://user1:password@127.0.0.1:5678/home/tmx/manifest.json";
	TmxBrokerContext _url1 {testA };

	BOOST_CHECK_EQUAL(_url1.to_string(), testA);
	BOOST_CHECK_EQUAL(_url1.get_scheme(), "explicit");
	BOOST_CHECK_EQUAL(_url1.get_user(), "user1");
	BOOST_CHECK_EQUAL(_url1.get_secret(), "password");
	BOOST_CHECK_EQUAL(_url1.get_host(), "127.0.0.1");
	BOOST_CHECK_EQUAL(_url1.get_port(), "5678");
	BOOST_CHECK_EQUAL(_url1.get_path(), "/home/tmx/manifest.json");

	// Check missing values
	const char *testB = "https://www.google.com/index.html";
	TmxBrokerContext _url2 {testB };

	BOOST_CHECK_EQUAL(_url2.to_string(), testB);
	BOOST_CHECK_EQUAL(_url2.get_scheme(), "https");
	BOOST_CHECK_EQUAL(_url2.get_user(), "");
	BOOST_CHECK_EQUAL(_url2.get_secret(), "");
	BOOST_CHECK_EQUAL(_url2.get_host(), "www.google.com");
	BOOST_CHECK_EQUAL(_url2.get_port(), "");
	BOOST_CHECK_EQUAL(_url2.get_path(), "/index.html");

	// User but no secret
	const char *testC = "myuser@example.com:8080";
    TmxBrokerContext _url3 {testC };

    BOOST_CHECK_EQUAL(_url3.to_string(), testC);
	BOOST_CHECK_EQUAL(_url3.get_scheme(), "");
	BOOST_CHECK_EQUAL(_url3.get_user(), "myuser");
	BOOST_CHECK_EQUAL(_url3.get_secret(), "");
	BOOST_CHECK_EQUAL(_url3.get_host(), "example.com");
	BOOST_CHECK_EQUAL(_url3.get_port(), "8080");
	BOOST_CHECK_EQUAL(_url3.get_path(), "");

	// Check the file, i.e. no host, case
	const char *testD = "file:///a/b/c/d.exe";
    TmxBrokerContext _url4 {testD };

    BOOST_CHECK_EQUAL(_url4.to_string(), testD);
	BOOST_CHECK_EQUAL(_url4.get_scheme(), "file");
	BOOST_CHECK_EQUAL(_url4.get_user(), "");
	BOOST_CHECK_EQUAL(_url4.get_secret(), "");
	BOOST_CHECK_EQUAL(_url4.get_host(), "");
	BOOST_CHECK_EQUAL(_url4.get_port(), "");
	BOOST_CHECK_EQUAL(_url4.get_path(), "/a/b/c/d.exe");

}


} /* namespace broker */
} /* namespace tmx */
