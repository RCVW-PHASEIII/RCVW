//==========================================================================
// Name        : ExamplePlugin.cpp
// Author      : Battelle Memorial Institute
// Version     :
// Copyright   : Copyright (c) 2014 Battelle Memorial Institute. All rights reserved.
// Description : Example Plugin
//==========================================================================

#include <tmx/plugin/TmxPlugin.hpp>
#include <tmx/message/TmxMessage.hpp>

#include "SampleData.h"

#include <chrono>
#include <thread>

using namespace tmx::common;
using namespace tmx::message;
using namespace tmx::plugin;

namespace tmx {
namespace plugin {
namespace example {

/**
 * This plugin is an example to demonstrate the capabilities of a TMX plugin.
 */
class ExamplePlugin: public TmxPlugin
{
public:
    /*!
     * @brief Default constructor
     *
     * The plugin is generally constructed in the main program, with no need
     * for arguments.
     */
	ExamplePlugin();

    /*!
     * @brief Default destructor
     *
     * Any non-standard allocations in the plugin must be cleaned up.
     */
	virtual ~ExamplePlugin();

    /*!
     * @brief Get the plugin descriptor
     *
     * The descriptor contains the instance pointer to this plugin, plus
     * a type identifier and a simple, non-qualified name for the plugin.
     *
     * @return The plugin descriptor
     */
    TmxTypeDescriptor get_descriptor() const noexcept override;

    /*!
     * @brief Execute the plugin
     *
     * Normally, this would set up the run-time environment, and then
     * just wait until the plugin is asked to stop, but it can be
     * customized. In this example, the plugin attempts to push out a
     * new SampleData message every 100 ms.
     *
     * @return The return code from the execution
     */
    TmxError main() override;

    /*!
     * @brief Return the sample data
     *
     * This plugin contains a custom data type.
     *
     * @return The sample data
     */
     inline auto get_data() const { return this->_data; }
//protected:
    /*!
     * @brief Handle an incoming SampleData message
     *
     * This is an example message handler. For this plugin, the data
     * is checked to determine how fast it was transmitted.
     *
     * @return The return code from the handler
     */
    TmxError handleSampleData(SampleData const &, message::TmxMessage const &);

	//void HandleMapDataMessage(MapDataMessage &msg, routeable_message &routeableMsg);
	//void HandleDecodedBsmMessage(DecodedBsmMessage &msg, routeable_message &routeableMsg);
	//void HandleDataChangeMessage(DataChangeMessage &msg, routeable_message &routeableMsg);

private:
    SampleData _data;
};

/**
 * Construct a new ExamplePlugin with the given name.
 *
 * @param name The name to give the plugin for identification purposes.
 */
ExamplePlugin::ExamplePlugin() {
    this->register_handler("Example/Data", this, &ExamplePlugin::handleSampleData);
}

ExamplePlugin::~ExamplePlugin() { }

TmxTypeDescriptor ExamplePlugin::get_descriptor() const noexcept {
    auto d = TmxPlugin::get_descriptor();
    return { d.get_instance(), typeid(ExamplePlugin), type_fqname(*this).data() };
}

TmxError ExamplePlugin::handleSampleData(SampleData const &data, message::TmxMessage const &msg) {
    message::TmxMessage tmp;
    tmp.set_timepoint();

    std::cout << "Received " << data.get_sequence() << " in " <<
        (tmp.get_timestamp() - msg.get_timestamp()) << " ns" << std::endl;
    return { };//this->broadcast(data, "Example/Data");
}


// Override of main method of the plugin that should not return until the plugin exits.
// This method does not need to be overridden if the plugin does not want to use the main thread.
TmxError ExamplePlugin::main() {
	std::cout << "Starting " << this->get_descriptor().get_type_name() << std::endl;

    auto last = std::chrono::system_clock::now();
	while (this->is_running()) {
        auto ret = this->broadcast(this->_data.get_memento(), "Example/Data", this->get_descriptor().get_type_name(), "json");
        if (ret)
            std::cerr << "ERROR: Unable to broadcast SampleData: " << ret.get_message() << std::endl;

		std::this_thread::sleep_for(std::chrono::milliseconds(100));

        auto now = std::chrono::system_clock::now();
        this->_data.increment_sequence();
        this->_data.increment_timer((now - last).count());
        last = now;
	}

	//PLOG(logINFO) << "Plugin terminating gracefully.";
	return { };
}

} /* End namespace example */
} /* End namespace plugin */
} /* End namespace tmx */

using namespace tmx::common;
using namespace tmx::common::types;
using namespace tmx::message::codec;

int main(int argc, char *argv[]) {
    example::ExamplePlugin plugin;
    return run(plugin, argc, argv);
}
