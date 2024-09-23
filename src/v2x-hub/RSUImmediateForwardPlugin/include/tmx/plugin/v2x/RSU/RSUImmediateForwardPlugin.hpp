/*!
 * Copyright (c) 2023 Battelle Memorial Institute
 *
 * All Rights Reserved.
 *
 * @file RSUImmediateForwardPlugin.hpp
 *
 *  Created on: Nov 18, 2023
 *      @author: Gregory M. Baumgardner
 */

#ifndef RSUIMMEDIATEFORWARDPLUGIN_HPP
#define RSUIMMEDIATEFORWARDPLUGIN_HPP

#include <tmx/common/TmxError.hpp>
#include <tmx/common/TmxTypeDescriptor.hpp>
#include <tmx/common/types/Any.hpp>
#include <tmx/plugin/TmxPlugin.hpp>
#include <tmx/plugin/TmxPluginDataUpdate.hpp>
#include <tmx/plugin/utils/FrequencyThrottle.hpp>

#include <atomic>
#include <cstdint>

namespace tmx {
namespace plugin {
namespace v2x {
namespace RSU {

class RSUImmediateForwardPlugin : public tmx::plugin::TmxPlugin {
    friend tmx::plugin::TmxPlugin;
public:
    RSUImmediateForwardPlugin();
    virtual ~RSUImmediateForwardPlugin() = default;

    common::TmxTypeDescriptor get_descriptor() const noexcept override;
    common::TmxError main() noexcept override;
    void on_message_received(message::TmxMessage const &) override;

protected:
    void init() noexcept override;
    common::types::Array<common::types::Any> get_config_description() const noexcept override;

private:
    // Message handlers
    void handle_config_update(TmxPluginDataUpdate const &, message::TmxMessage const &);
    void handle_incoming(common::types::Any const &, message::TmxMessage const &);

    // Mutex along with the data it protects.
    std::mutex _dataLock;

    std::atomic<bool> _muteRadio;
    std::atomic<std::uint64_t> _recvMsgs { 0 };
    std::atomic<std::uint64_t> _sentMsgs { 0 };
    utils::FrequencyThrottle<int> _statusThrottle;
};

} /* End namespace RSU */
} /* End namespace v2x */
} /* End namespace plugin */
} /* End namespace tmx */

#endif /* RSUIMMEDIATEFORWARDPLUGIN_HPP */
