/*!
 * Copyright (c) 2021 Battelle Memorial Institute
 *
 * All Rights Reserved.
 *
 * @file TmxClient_test.cpp
 *
 *  Created on: Aug 11, 2021
 *      @author: Rich Surgenor
 */

#include <tmx/plugin/TmxPlugin.hpp>
#include <tmx/common/TmxException.hpp>

using namespace tmx;
// using namespace tmx::broker;
using namespace tmx::common::types;
using namespace tmx::plugin;

// namespace tmx {
// namespace plugin {


// } /* namespace broker */
// } /* namespace tmx */

// main tester in root_tmx_dir/test/TmxPlugin_test.cpp because I couldn't figure out
// how to get all deps otherwise

using string = tmx::common::types::string;
using any = tmx::common::any;

int main(int argsc, char** argsv) {
    printf("hii\n")
    std::string url("todo");
    // const char* url = "todo";
    TmxPlugin* plugin = TmxPlugin::initialize(std::move(url));

    string host("127.0.0.1");
    string client("what");
    string group("0");
    const any error;
    common::types::uint16 port = 9001;
    plugin->connect(host, std::move(port), client, group, std::move(error));
    // plugin->_broker->setup(&host, &port, &_client, &group);
    while (true)
    {
        printf("waiting..\n");
        sleep(10);
    }
}