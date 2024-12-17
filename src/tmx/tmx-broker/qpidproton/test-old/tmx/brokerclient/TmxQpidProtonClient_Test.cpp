/*!
 * Copyright (c) 2023 Battelle Memorial Institute
 *
 * All Rights Reserved.
 *
 * @file TmxBrokerClient_test.cpp
 *
 *  Created on: Jul 19, 2023
 *      @author: Nate Clark
 */

#include <tmx/broker/TmxBrokerContext.hpp>
#include <tmx/broker/qpidproton/TmxQpidProtonClient.hpp>

#include <boost/test/unit_test.hpp>
#include <fstream>

using namespace tmx::broker;

extern void test_tmx_broker(TmxBrokerContext &, std::map<std::string, std::string>);

namespace tmx {
namespace broker {
namespace qpidproton {

    BOOST_AUTO_TEST_CASE (test_qpid_proton_broker) {


        TmxBrokerContext ctx { "sb://servername", "qpid-proton-broker" };

        test_tmx_broker(ctx, { { } });
        TmxQpidProtonClient qpidProtonClient;

//        static uint32_t  messagesReceived = 0L;
//        uint32_t messagesSent = 0L;
//
//        std::function<common::TmxError(const std::any&, const tmx::message::TmxMessage&)> fms = [] (const std::any& id, const tmx::message::TmxMessage& msg) {
//            common::TmxError tmxError;
//            messagesReceived++;
//            if ( 0 == messagesReceived % 100 )
//                std::cout << "Received " << messagesReceived << std::endl;
//            return tmxError;
//        };
//
//        common::TmxTypeDescriptor cycloneCallback(std::shared_ptr<const void>(&fms),typeid(fms),"TmxQpidProtonClient-receiveMessage");
//
//        qpidProtonClient.initialize(ctx);
//
//        qpidProtonClient.connect(ctx);
//
//        char *pszTopic = getenv("MSG_TOPIC");
//        char szInput[1024];
//
//        qpidProtonClient.subscribe(ctx,pszTopic,cycloneCallback);
//
//        std::ifstream inFile(getenv("MSG_SOURCE_FILE"));
//
//        TmxMessage helloWorld;
//        helloWorld.set_metadata(0);
//        helloWorld.set_topic(pszTopic);
//
//        static long sequence = 0;
//
//        while ( ! inFile.eof() ) {
//            inFile.getline(szInput, 1024);
//            helloWorld.set_payload(common::to_byte_sequence(szInput,strlen(szInput)));
//            helloWorld.set_metadata(sequence++);
//            qpidProtonClient.publish(ctx, helloWorld);
//            messagesSent++;
//            if ( 0 == messagesSent % 100 )
//                std::cout << "Sent " << messagesSent << std::endl;
//if ( 100 == messagesSent )
//break;
//        }
//
//        //qpidProtonClient.show_statistics(ctx);
//
//#if 0
//        qpidProtonClient.disconnect(ctx);
//#else
//        qpidProtonClient.unsubscribe(ctx,pszTopic,cycloneCallback);
//#endif
//
//        BOOST_CHECK_EQUAL(messagesSent,messagesReceived);

    }

} /* namespace qpidproton */
} /* namespace broker */
} /* namespace tmx */
