/*!
* Copyright (c) 2023 Battelle Memorial Institute
*
* All Rights Reserved.
*
* @file DifferentialGPSPlugin.hpp
*
*  Created on: May 30, 2024
*      @author: Noah Limes
*/

#ifndef SRC_DIFFERENTIALGPSPLUGIN_HPP_
#define SRC_DIFFERENTIALGPSPLUGIN_HPP_

#include <atomic>
#include <mutex>
#include <string>
#include <cstdint>

#include <tmx/message/v2x/LocationMessage.hpp>
#include <tmx/message/v2x/rtcm/RtcmMessage.hpp>

#include <tmx/common/types/Any.hpp>
#include <tmx/common/TmxError.hpp>
#include <tmx/common/TmxTypeDescriptor.hpp>

#include <tmx/message/TmxData.hpp>
#include <tmx/message/TmxMessage.hpp>

#include <tmx/plugin/TmxPlugin.hpp>
#include <tmx/plugin/TmxPluginDataUpdate.hpp>
#include <tmx/plugin/utils/FrequencyThrottle.hpp>

namespace tmx {
namespace plugin {
namespace v2x {
namespace differentialGPS {

class DifferentialGPSPlugin : public tmx::plugin::TmxPlugin {
public:
    DifferentialGPSPlugin();
    ~DifferentialGPSPlugin();

    common::TmxTypeDescriptor get_descriptor() const noexcept override;

    void set_config(common::const_string, const common::types::Any &, std::mutex * = nullptr) override;

    void handle_location(tmx::message::TmxData const &data, tmx::message::TmxMessage const &msg);
    void handle_rtcm(tmx::message::TmxData const &data, tmx::message::TmxMessage const &msg);

    void on_message_received(const message::TmxMessage &) override;

protected:
    common::types::Array<common::types::Any> get_config_description() const noexcept override;

private:
    //Tags for the handlers
    struct on_config_update {};
    struct on_location {};
    struct on_rtcm {};

    //Members
    std::atomic<float> _latchSpeed{0};

    std::atomic<int> _msgCount {0};
    std::atomic<int> _byteCount {0};

    std::atomic<bool> _doWrite {true};

    std::mutex _cfgLock;
    std::mutex _writeLock;

    std::atomic<std::uint64_t> _sentMsgs{0};

    //Frequency throttles
    utils::FrequencyThrottle<int> _statusThrottle;
    utils::FrequencyThrottle<int> _errThrottle;

};

} /* End namespace DifferentialGPS */
} /* End namespace rcvw */
} /* End namespace plugin */
} /* End namespace tmx */

#endif /* SRC_DIFFERENTIALGPSPLUGIN_HPP_ */
