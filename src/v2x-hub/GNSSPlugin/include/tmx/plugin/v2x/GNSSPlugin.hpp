/*!
 * Copyright (c) 2023 Battelle Memorial Institute
 *
 * All Rights Reserved.
 *
 * @file GNSSPlugin.hpp
 *
 *  Created on: Jul 30, 2020
 *      @author: Gregory M. Baumgardner
 *  Edited on: May 8, 2024
 *      @author: Noah Limes
 */

#ifndef SRC_GNSSPLUGIN_HPP_
#define SRC_GNSSPLUGIN_HPP_

#include <atomic>
#include <mutex>
#include <string>
#include <cstdint>

#include <tmx/message/v2x/LocationMessage.hpp>
#include <tmx/message/v2x/NmeaMessage.hpp>

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
namespace GNSS {

class GNSSPlugin : public tmx::plugin::TmxPlugin {
public:
    GNSSPlugin();
    virtual ~GNSSPlugin() = default;

    common::TmxTypeDescriptor get_descriptor() const noexcept override;
    void handle_config_update(TmxPluginDataUpdate const &, tmx::message::TmxMessage const &);
    common::TmxError main() noexcept override;

    // GPSD Handlers
    void handle_raw_message(tmx::common::types::String8 const &data, tmx::message::TmxMessage const &msg);
    void handle_gpsd_message(tmx::message::v2x::LocationMessage const &data, tmx::message::TmxMessage const &msg);
    void handle_nmea_message(tmx::message::v2x::NmeaMessage const &data, tmx::message::TmxMessage const &msg);

protected:
    common::types::Array<common::types::Any> get_config_description() const noexcept override;

private:

    //Tags for handlers
    struct check_status { };
    struct on_nmea { };
    struct on_raw { };
    struct on_gpsd { };

    //Members
    tmx::message::v2x::LocationMessage _locationMsg;
    std::mutex _locationLock;
    std::atomic<bool> _diffStat_checked { false };


    std::atomic<bool> _sendNMEA { false };
    std::atomic<float> _latchSpeed { 0 };

    std::atomic<bool> _updateGPSDConfigFlag { false };
    common::types::_float_t _updateGPSDConfigDelay { 5.0 };

    std::atomic<std::uint64_t> _recvNMEA{ 0};
    std::atomic<std::uint64_t> _recvFormattedGPSD{ 0};
    std::atomic<std::uint64_t> _sentMsgs { 0 };

    common::types::Array<common::types::Any> _NMEAFilters { }; //GGA, PPA, GGV, RMC, VTG, GLL, GSA

    std::mutex _dataLock;

    //Frequency throttles
    utils::FrequencyThrottle<int> _statusThrottle;
    utils::FrequencyThrottle<int> _locationThrottle;
    utils::FrequencyThrottle<int> _errThrottle;


};

} /* End namespace GNSS */
} /* End namespace v2x */
} /* End namespace plugin */
} /* End namespace tmx */

#endif /* SRC_GNSSPLUGIN_HPP_ */
