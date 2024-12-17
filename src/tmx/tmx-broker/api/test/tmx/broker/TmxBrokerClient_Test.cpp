/*!
 * Copyright (c) 2023 Battelle Memorial Institute
 *
 * All Rights Reserved.
 *
 * @file TmxBrokerClient_Test.cpp
 *
 *  Created on: Jul 17, 2023
 *      @author: Nate Clark
 */

#include <tmx/broker/TmxBrokerClient.hpp>
#include <tmx/broker/TmxBrokerContext.hpp>
#include <tmx/common/TmxLogger.hpp>

#include <boost/test/unit_test.hpp>

using namespace tmx::common;
using namespace tmx::message;

namespace tmx {
namespace broker {

void test_tmx_broker(TmxBrokerContext &, std::map<std::string, std::string> = {});

void test_tmx_broker(TmxBrokerContext &ctx, std::map<std::string, std::string> checks) {
    BOOST_CHECK(ctx.get_state() == TmxBrokerState::uninitialized);

    auto client = TmxBrokerClient::get_broker(ctx);

    // Make sure a client was found, and has a valid descriptor
    BOOST_CHECK(client.operator bool());
    if (!client)
        return;

    BOOST_CHECK(client->get_descriptor());
    BOOST_CHECK(client->get_descriptor().get_type_name().length());
    BOOST_CHECK(!client->is_connected(ctx));
    BOOST_CHECK(!client->is_subscribed(ctx, "test-callback"));

    typedef types::Properties<types::Any> map_type;
    typedef typename map_type::key_t key_t;

    auto info = client->get_broker_info(ctx);
    auto map = types::as_properties<typename map_type::value_t>(info);

    BOOST_CHECK(map);
    if (!map)
        return;

    checks.insert({
            {"scheme", ctx.get_scheme().c_str()},
            {"user", ctx.get_user().c_str()},
            {"secret", ctx.get_secret().c_str()},
            {"host", ctx.get_host().c_str()},
            {"port", ctx.get_port().c_str()},
            {"path", ctx.get_path().c_str()}
    });

    for (auto const &kv: checks) {
        key_t key { kv.first };
        BOOST_CHECK(map->count(key));
        if (!map->count(key))
            continue;

        auto str = types::as_string8(map->at(key));
        if (str) {
            BOOST_CHECK_EQUAL(kv.second, str->c_str());
            continue;
        }

        auto b = types::as_bool(map->at(key));
        if (b) {
            BOOST_CHECK_EQUAL(kv.second, *b ? "true" : "false");
            continue;
        }

        auto i = types::as_int(map->at(key));
        if (i) {
            BOOST_CHECK_EQUAL(kv.second, std::to_string(*i));
            continue;
        }

        auto f = types::as_double(map->at(key));
        if (f) {
            BOOST_CHECK_EQUAL(kv.second, std::to_string(*f));
            continue;
        }

        BOOST_CHECK(kv.second == "null");
    }

    client->initialize(ctx);
    sleep(5);
    BOOST_CHECK(ctx.get_state() == TmxBrokerState::initialized);

    client->connect(ctx);
    sleep(5);
    BOOST_CHECK(ctx.get_state() == TmxBrokerState::connected);

    client->disconnect(ctx);
    sleep(5);
    BOOST_CHECK(ctx.get_state() == TmxBrokerState::disconnected);

    client->connect(ctx);
    sleep(5);
    BOOST_CHECK(ctx.get_state() == TmxBrokerState::connected);

    client->destroy(ctx);
    sleep(5);
    BOOST_CHECK(ctx.get_state() == TmxBrokerState::uninitialized);
}

BOOST_AUTO_TEST_CASE ( test_url_parse ) {
    // Check fully expanded case
    const char *testA = "explicit://user1:password@127.0.0.1:5678/home/tmx/manifest.json";
    TmxBrokerContext _url1 {testA };

    BOOST_CHECK_EQUAL(_url1.to_string().c_str(), testA);
    BOOST_CHECK_EQUAL(_url1.get_scheme(), "explicit");
    BOOST_CHECK_EQUAL(_url1.get_user(), "user1");
    BOOST_CHECK_EQUAL(_url1.get_secret(), "password");
    BOOST_CHECK_EQUAL(_url1.get_host(), "127.0.0.1");
    BOOST_CHECK_EQUAL(_url1.get_port(), "5678");
    BOOST_CHECK_EQUAL(_url1.get_path(), "/home/tmx/manifest.json");

    // Check missing values
    const char *testB = "https://www.google.com/index.html";
    TmxBrokerContext _url2 {testB };

    BOOST_CHECK_EQUAL(_url2.to_string().c_str(), testB);
    BOOST_CHECK_EQUAL(_url2.get_scheme(), "https");
    BOOST_CHECK_EQUAL(_url2.get_user(), "");
    BOOST_CHECK_EQUAL(_url2.get_secret(), "");
    BOOST_CHECK_EQUAL(_url2.get_host(), "www.google.com");
    BOOST_CHECK_EQUAL(_url2.get_port(), "");
    BOOST_CHECK_EQUAL(_url2.get_path(), "/index.html");

    // User but no secret
    const char *testC = "smtp://myuser@example.com:8080";
    TmxBrokerContext _url3 { testC };

    BOOST_CHECK_EQUAL(_url3.to_string().c_str(), testC);
    BOOST_CHECK_EQUAL(_url3.get_scheme(), "smtp");
    BOOST_CHECK_EQUAL(_url3.get_user(), "myuser");
    BOOST_CHECK_EQUAL(_url3.get_secret(), "");
    BOOST_CHECK_EQUAL(_url3.get_host(), "example.com");
    BOOST_CHECK_EQUAL(_url3.get_port(), "8080");
    BOOST_CHECK_EQUAL(_url3.get_path(), "");

    // Check the file, i.e. no host, case
    const char *testD = "file://localhost/a/b/c/d.exe";
    TmxBrokerContext _url4 { testD };

    BOOST_CHECK_EQUAL(_url4.to_string().c_str(), testD);
    BOOST_CHECK_EQUAL(_url4.get_scheme(), "file");
    BOOST_CHECK_EQUAL(_url4.get_user(), "");
    BOOST_CHECK_EQUAL(_url4.get_secret(), "");
    BOOST_CHECK_EQUAL(_url4.get_host(), "localhost");
    BOOST_CHECK_EQUAL(_url4.get_port(), "");
    BOOST_CHECK_EQUAL(_url4.get_path(), "/a/b/c/d.exe");

}

#ifdef TMXTEST_AMQP_CONTEXT
BOOST_AUTO_TEST_CASE(test_qpidproton_client) {
    common::TmxLogger::enable(common::TmxLogLevel::DEBUG3);

    TmxBrokerContext context { TMXTEST_AMQP_CONTEXT, "qpid-proton-broker" };
    test_tmx_broker(context);
}
#endif

/*   BOOST_AUTO_TEST_CASE (test_broker_client) {

        //NTC: As far as I can tell, DDS does not need a "server", I cannot find in any
        // documention for DDS or CycloneDDS ANYWHERE where it references some message server/broker IP,
        // connection, or ANYTHING !!!??? If it's intended to publish/subscribe on the same machine - why
        // the focus on NetworkInterface in the documentation ????

        common::types::Any threadCount = std::map<std::string,common::types::Any> ( { {"deliveryThreads", (unsigned int)1} } );
        TmxBrokerContext ctx("explicit://nate:pubfly@127.0.0.1",threadCount);
        TmxCycloneClient cycloneClient;

        static uint32_t  messagesReceived = 0L;
        uint32_t messagesSent = 0L;

        std::function<common::TmxError(const std::any&, const tmx::message::TmxMessage&)> fms = [] (const std::any& id, const tmx::message::TmxMessage& msg) {
            common::TmxError tmxError;
            messagesReceived++;
            if ( 0 == messagesReceived % 100 )
                std::cout << "Received " << messagesReceived << std::endl;
            return tmxError;
        };

        common::TmxTypeDescriptor cycloneCallback(std::shared_ptr<const void>(&fms),typeid(fms),"TmxCycloneClient-receiveMessage");

        cycloneClient.initialize(ctx);

        cycloneClient.connect(ctx);

        char *pszTopic = getenv("MSG_TOPIC");
        char szInput[1024];

time_t theTime;
time(&theTime);
        cycloneClient.subscribe(ctx,pszTopic,cycloneCallback);

        std::ifstream inFile(getenv("MSG_SOURCE_FILE"));

        TmxMessage helloWorld;
        helloWorld.set_metadata(0);
        helloWorld.set_topic(pszTopic);

        static long sequence = 0;

        while ( ! inFile.eof() ) {
            inFile.getline(szInput, 1024);
            helloWorld.set_payload(common::to_byte_sequence(szInput,strlen(szInput)));
            helloWorld.set_metadata(sequence++);

// without throttling the sending speed, I CANNOT get the
// reader to NOT drop messages (!?!)
// NOR can I find a decent explanation of how to set parameters (QoS)
// to setup "Reliable Communications"
// If we ever actually use DDS - we'll figure it out
usleep(50000);
            cycloneClient.publish(ctx, helloWorld);
            messagesSent++;
            if ( 0 == messagesSent % 100 )
                std::cout << "Sent " << messagesSent << std::endl;

if ( 100 == messagesSent )
break;
        }

        cycloneClient.unsubscribe(ctx,pszTopic,cycloneCallback);

        BOOST_CHECK_EQUAL(messagesSent,messagesReceived);
time_t endTime;
time(&endTime);

std::cout << "RunTime " << endTime - theTime << " seconds" << std::endl;

    }
*/
} /* namespace broker */
} /* namespace tmx */
