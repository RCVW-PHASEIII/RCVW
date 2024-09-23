/*!
 * Copyright (c) 2023 Battelle Memorial Institute
 *
 * All Rights Reserved.
 *
 * @file EmptyPlugin.cpp
 *
 *  Created on: Mar 2, 2023
 *      @author: Gregory M. Baumgardner
 */

#include <tmx/plugin/TmxPlugin.hpp>

using namespace tmx::common;
using namespace tmx::plugin;


namespace tmx {
namespace plugin {
namespace example {

class EmptyPlugin : public TmxPlugin {
public:
    EmptyPlugin();
    virtual ~EmptyPlugin();

    TmxError main() override;

protected:
    void reinit() override;

    TmxError on_config_update(types::Any const &) override;
    TmxError on_status_update(types::Any const &) override;
    TmxError on_error(types::Any const &) override;
};

EmptyPlugin::EmptyPlugin() {
}

EmptyPlugin::~EmptyPlugin() {
}

TmxError EmptyPlugin::main() {
    return { };
}

void EmptyPlugin::reinit() {
}

TmxError EmptyPlugin::on_config_update(types::Any const &) {
    return { };
}

TmxError EmptyPlugin::on_status_update(types::Any const &) {
    return { };
}

TmxError EmptyPlugin::on_error(types::Any const &) {
    return { };
}

} /* End namespace example */
} /* End namespace plugin */
} /* End namespace tmx */

int main(int argc, char *argv[]) {
    example::EmptyPlugin plugin;
	return tmx::plugin::run(plugin, argc, argv);
}
