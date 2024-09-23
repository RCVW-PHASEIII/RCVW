/*!
 * Copyright (c) 2016 Battelle Memorial Institute
 *
 * All Rights Reserved.
 *
 * @file GNSSPlugin.cpp
 *
 *  Created on: Jul 30, 2020
 *      @author: Gregory M. Baumgardner
 *  Edited on: May 5, 2024
 *      @author: Noah Limes
 */

#include <tmx/plugin/v2x/GNSS_Configuration.hpp>
#include <tmx/plugin/v2x/GNSSPlugin.hpp>

#include <tmx/plugin/TmxChannel.hpp>
#include <tmx/plugin/utils/Clock.hpp>

#include <tmx/broker/TmxBrokerContext.hpp>
#include <tmx/broker/TmxBrokerClient.hpp>

#include <tmx/common/TmxLogger.hpp>

#include <tmx/message/codec/TmxCodec.hpp>
#include <tmx/message/codec/serializer/TmxDataSerializer.hpp>

using namespace std;
using namespace tmx::common;
using namespace tmx::message;
using namespace tmx::common::types;
using namespace tmx::plugin::utils;
using namespace tmx::message::codec::serializer;

namespace tmx {
namespace plugin {
namespace v2x {
namespace GNSS {

static constexpr auto _gps_src = "GPSSource";
static constexpr auto _send_nmea = "SendRawNMEA";
static constexpr auto _latch_speed = "LatchSpeed";

/**
 * Default Constructor. Good Place to initialize atomic variables.
 *
 * @param name string used to identify the plugin.
 */
GNSSPlugin::GNSSPlugin() {

    //Private tags for the handlers
    struct on_config_update { };
    struct handle_incoming { };

    //Set Throttles
    this->_statusThrottle.set_Frequency(std::chrono::seconds(1));

    //Register handlers
    for (TmxData param : this->get_config_description()) {
        this->register_handler<on_config_update>(this->get_topic("/config/" + param["key"].to_string()), this,
                                                 &GNSSPlugin::handle_config_update);
    }
}

TmxTypeDescriptor GNSSPlugin::get_descriptor() const noexcept {
    auto _descr = TmxPlugin::get_descriptor();
    return { _descr.get_instance(), typeid(GNSSPlugin), "GNSSPlugin" };
}

types::Array<types::Any> GNSSPlugin::get_config_description() const noexcept {
    message::TmxData _data;
    auto decoder = message::codec::TmxDecoder::get_decoder("json");
    if (decoder)
        decoder->decode(_data.get_container(), to_byte_sequence(GNSS_CONFIGURATION_JSON));
    return _data.to_array();
}

void GNSSPlugin::handle_config_update(TmxPluginDataUpdate const &data, message::TmxMessage const &) {
    TLOG(DEBUG) << "handle_config_updated invoked with " << data.get_container();

    auto str = data.get_key();
    if (str.empty()) {
        this->broadcast<TmxError>({ 1, "Invalid configuration update message: Missing key." }, this->get_topic("error"),
                                  __FUNCTION__);
        return;
    }

    if (data.get_value().is_empty()) {
        this->broadcast<TmxError>({ 2, "Invalid configuration update message: Missing new value." },
                                  this->get_topic("error"),
                                  __FUNCTION__);
        return;
    }

    if (strcmp("STATUS-HZ", str.c_str()) == 0) {
        std::lock_guard<std::mutex> _lock(this->_dataLock);
        _statusThrottle.set_Frequency(chrono::milliseconds(data.get_value().to_uint()));

        TLOG(DEBUG) << "STATUS-HZ set to " <<
                    chrono::duration_cast<chrono::milliseconds>(_statusThrottle.get_Frequency()).count() << " ms";
    }

    if (strcmp("LOCATION-HZ", str.c_str()) == 0) {
        std::lock_guard<std::mutex> _lock(this->_dataLock);
        _locationThrottle.set_Frequency(chrono::milliseconds(data.get_value().to_uint()));
        TLOG(DEBUG) << "LOCATION-HZ set to " <<
                    chrono::duration_cast<chrono::milliseconds>(_locationThrottle.get_Frequency()).count() << " ms";
    }

    if (strcmp("SEND-NMEA", str.c_str()) == 0) {
        this->_sendNMEA = data.get_value().to_bool();
        TLOG(DEBUG) << "SEND-NMEA set to " << this->_sendNMEA;
    }

    if (strcmp("LATCH-SPEED", str.c_str()) == 0) {
        this->_latchSpeed = data.get_value().to_float();
        TLOG(DEBUG) << "LATCH-SPEED set to " << this->_latchSpeed;
    }

    if (strcmp("NMEA-FILTERS", str.c_str()) == 0) {
        this->_NMEAFilters = data.get_value().to_array();
        TLOG(DEBUG) << "NMEA-FILTERS set to " << this->_NMEAFilters;
    }

    else {
        _updateGPSDConfigFlag = true;
        //trigger asyncio timer for _updateGPSDConfigDelay seconds,
        // at which point actually update config all gpsd-config values at once
        // or would you trigger it via some "Finalize Config Changes" topic?
    }

}

void GNSSPlugin::handle_raw_message(const tmx::common::types::String8 &ubxBytes, const tmx::message::TmxMessage &msg) {

    std::lock_guard<std::mutex> lock(_locationLock);

    // determine is float or fixed from UBX NAV-PVT message's 21st byte
    // according to u-blox F9 HPS 1.20 Interface description
    if (ubxBytes.size() >= 21 && (ubxBytes[0] == 0xB5 && ubxBytes[1] == 0x62 && ubxBytes[2] == 0x01 && ubxBytes[3] == 0x07)) {
        uint8_t fixStatusFlags = ubxBytes[20];
        if (fixStatusFlags & 0x02) //diffSoln, second bit 1
            _locationMsg.set_SignalQuality(message::v2x::SignalQualityTypes::RealTimeKinematic);
        else //diffSoln, second bit 0
            _locationMsg.set_SignalQuality(message::v2x::SignalQualityTypes::FloatRTK);
        _diffStat_checked = true;
    }

}

void GNSSPlugin::handle_gpsd_message(tmx::message::v2x::LocationMessage const &data, tmx::message::TmxMessage const &msg) {

    std::lock_guard<std::mutex> lock(_locationLock);

    _locationMsg = data;
    _diffStat_checked=false;

}

void GNSSPlugin::handle_nmea_message(tmx::message::v2x::NmeaMessage const &data, tmx::message::TmxMessage const &msg) {

    // passthrough
    if (_send_nmea)
        this->broadcast(msg, "V2X/NMEA");
}

/**
 * Main Function logic to execute on a separate thread
 *
 * @return exit code
 */
TmxError GNSSPlugin::main() noexcept {
    this->set_status("State", "Running");

    TmxMessage msg;
    msg.set_topic(type_fqname<check_status>().data());

    TLOG(DEBUG) << "Main thread is " << std::this_thread::get_id();

    while(this->is_running()) {

        if (_locationThrottle.Monitor(1) && _diffStat_checked) {
            this->broadcast(_locationMsg, "V2X/Location");
        }

    }

    this->set_status("State", "Terminated");

    return {};
}

} /* namespace GNSS */
} /* namespace v2x */
} /* namespace plugin */
} /* namespace tmx */

int main(int argc, char* argv[])
{
	tmx::plugin::v2x::GNSS::GNSSPlugin _plugin;
    return run(_plugin, argc, argv);
}
