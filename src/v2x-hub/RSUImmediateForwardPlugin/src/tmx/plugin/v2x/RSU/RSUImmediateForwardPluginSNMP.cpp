/*!
 * Copyright (c) 2023 Battelle Memorial Institute
 *
 * All Rights Reserved.
 *
 * @file RSUImmediateForwardPlugin.cpp
 *
 *  Created on: Nov 18, 2023
 *      @author: Gregory M. Baumgardner
 */

#include <tmx/plugin/v2x/RSU/RSUImmediateForwardPlugin.hpp>

#include <tmx/broker/TmxBrokerClient.hpp>
#include <tmx/common/TmxLogger.hpp>
#include <tmx/message/codec/serializer/TmxDataSerializer.hpp>
#include <tmx/plugin/TmxChannel.hpp>

#include <iostream>

using namespace std;
using namespace tmx;
using namespace tmx::common;
using namespace tmx::message;
using namespace tmx::message::codec::serializer;
using namespace tmx::plugin::utils;

namespace tmx {
namespace plugin {
namespace v2x {
namespace RSU {

struct handle_rsu_mib {
    static constexpr const char *topic = "snmpget/RSU-MIB/rsuMIB";
    static constexpr const char *oid_payload[] = {
        "RSU-MIB::rsuMibVersion",
        "RSU-MIB::rsuFirmwareVersion",
        "RSU-MIB::rsuLocationDesc",
        "RSU-MIB::rsuID",
        "RSU-MIB::rsuManufacturer",
        "RSU-MIB::rsuTxPower",
        "RSU-MIB::rsuMode"
    };
};

struct handle_msg_stats {
    static constexpr const char *topic = "snmpget/RSU-MIB/rsuSystemStats";
    static constexpr const char *oid_payload[] = {
        "RSU-MIB::rsuAltSchMsgSent",
        "RSU-MIB::rsuAltSchMsgRcvd",
        "RSU-MIB::rsuAltCchMsgSent",
        "RSU-MIB::rsuAltCchMsgRcvd",
        "RSU-MIB::rsuContSchMsgSent",
        "RSU-MIB::rsuContSchMsgRcvd",
        "RSU-MIB::rsuContCchMsgSent",
        "RSU-MIB::rsuContCchMsgRcvd"
    };
};

struct handle_ifm_mib {
    static constexpr const char *topic = "snmpget/RSU-MIB/rsuIFMStatusTable";
    static constexpr const char *oid_payload[] = {
            "RSU-MIB::rsuIFMPsid",
            "RSU-MIB::rsuIFMDsrcMsgId",
            "RSU-MIB::rsuIFMTxMode",
            "RSU-MIB::rsuIFMTxChannel",
            "RSU-MIB::rsuIFMEnable",
            "RSU-MIB::rsuIFMStatus"
    };
};

struct handle_incoming { };

template <typename _Tp>
void next_request(_Tp const &t, TmxPlugin *plugin, std::uint8_t idx = 0) {
    codec::TmxCodec codec;
    codec.get_message().set_topic(_Tp::topic);
    codec.get_message().set_timepoint();

    TmxData data;
    for (const auto &oid: _Tp::oid_payload)
        data[std::string(oid) + "." + std::to_string(idx)] = true;

    auto err = codec.encode(data.get_container(), "json");
    if (err)
        plugin->broadcast<TmxError>(err, plugin->get_topic("error"));
    else
        plugin->broadcast(codec.get_message());
}


std::string get_message_topic_name(const_string msgType) {
    static TmxTypeRegistry j2735_reg { "tmx.message.J2735" };

    auto descr = j2735_reg.get("get-message-name");
    auto fn = descr.as_instance<std::function<std::string(std::string)> >();
    if (fn)
        return (*fn)(std::string(msgType.data(), msgType.length()));
    else
        return "";
}

// Different time constraints for the different status information
static FrequencyThrottle<std::string> _updateThrottle;
static FrequencyThrottle<int> _cfgThrottle;

} /* End namespace RSU */
} /* End namespace v2x */

template <>
void TmxPlugin::on_message_received<TmxPluginDataUpdate const>(TmxPluginDataUpdate const &upd,
                                                               message::TmxMessage const &msg) {
    TLOG(DEBUG) << std::this_thread::get_id() << ": Enter " << TMX_PRETTY_FUNCTION << " with " << msg.to_string();

    if (upd.get_old_value().to_string() == upd.get_new_value().to_string())
        return;

    for (auto rsu: upd.get_new_value().to_map()) {
        TmxData const ifmTable{ rsu.second };

        for (std::size_t i = 0; i < (std::size_t) ifmTable; i++) {
            auto const &rsuInfo = ifmTable[i];

            if (rsuInfo["ID"]) {
                auto topic = v2x::RSU::get_message_topic_name(rsuInfo["ID"].to_string());
                if (!topic.empty())
                    this->register_handler<v2x::RSU::handle_incoming>("J2735/" + topic,
                                                                      dynamic_cast<v2x::RSU::RSUImmediateForwardPlugin *>(this),
                                                                      &v2x::RSU::RSUImmediateForwardPlugin::handle_incoming);
            }
        }
    }

    if (std::strcmp("Initializing", this->get_status("State").to_string().c_str()))
        TmxPlugin::init();
}

// Threaded request
template <>
void TmxPlugin::on_message_received<TmxData const, std::true_type>(TmxData const &, TmxMessage const &msg) {
    TLOG(DEBUG3) << std::this_thread::get_id() << ": Enter " << TMX_PRETTY_FUNCTION << " with " << msg.to_string();

    auto plugin = dynamic_cast<v2x::RSU::RSUImmediateForwardPlugin *>(this);
    if (!plugin)
        return;

    static const char *topics[] = { v2x::RSU::handle_ifm_mib::topic,
                                    v2x::RSU::handle_rsu_mib::topic,
                                    v2x::RSU::handle_msg_stats::topic };

    if (v2x::RSU::_cfgThrottle.Monitor(0))
        v2x::RSU::next_request(v2x::RSU::handle_ifm_mib{ }, this, 1);

    // Only update those RSUs that are already detected
    TmxData const &RSUs = this->get_config("RSUs", &(plugin->_dataLock));

//    for (auto rsu: RSUs.to_map()) {
//        if (v2x::RSU::_updateThrottle.Monitor(rsu.first)) {
//            v2x::RSU::next_request(v2x::RSU::handle_rsu_mib{ }, this);
//            v2x::RSU::next_request(v2x::RSU::handle_msg_stats{ }, this);
//        }
//    }
}

// The SNMP handlers
template <>
void TmxPlugin::on_message_received<TmxData const, v2x::RSU::handle_rsu_mib>(TmxData const &data,
                                                                             TmxMessage const &msg) {
    TLOG(DEBUG) << std::this_thread::get_id() << ": Enter " << TMX_PRETTY_FUNCTION << " with " << msg.to_string();

    auto plugin = dynamic_cast<v2x::RSU::RSUImmediateForwardPlugin *>(this);
    if (!plugin)
        return;

    // Get the host information
    TmxData rsuInfo;

    for (auto const &kv: data.to_map()) {
        auto nm = kv.first.substr(strlen("RSU-MIB::rsu"));
        if (std::strncmp("MibVersion", nm.c_str(), 10) == 0)
            rsuInfo[msg.get_source()]["version"] = kv.second;
        if (std::strncmp("Manufacturer", nm.c_str(), 12) == 0)
            rsuInfo[msg.get_source()]["make"] = kv.second;
        if (std::strncmp("FirmwareVersion", nm.c_str(), 15) == 0)
            rsuInfo[msg.get_source()]["firmware"] = kv.second;
        if (std::strncmp("ID", nm.c_str(), 2) == 0)
            rsuInfo[msg.get_source()]["ID"] = kv.second;
        if (std::strncmp("Mode", nm.c_str(), 4) == 0)
            rsuInfo[msg.get_source()]["mode"] = kv.second;
        if (std::strncmp("Power", nm.c_str(), 12) == 0)
            rsuInfo[msg.get_source()]["power"] = kv.second;
    }

    this->set_status("RSU Info", rsuInfo.get_container());
}

template <>
void TmxPlugin::on_message_received<TmxData const, v2x::RSU::handle_msg_stats>(TmxData const &data,
                                                                               TmxMessage const &msg) {
    TLOG(DEBUG) << std::this_thread::get_id() << ": Enter " << TMX_PRETTY_FUNCTION << " with " << msg.to_string();

    auto plugin = dynamic_cast<v2x::RSU::RSUImmediateForwardPlugin *>(this);
    if (!plugin)
        return;

    // Get the message statistics
    typename common::types::UIntmax::value_type sent = 0;
    typename common::types::UIntmax::value_type rcvd = 0;

    for (auto const &kv: data.to_map()) {
        auto nm = kv.first.substr();
        if (kv.first.find("MsgSent") < kv.first.length())
            sent += TmxData(kv.second).to_uint();
        if (kv.first.find("MsgRcvd") < kv.first.length())
            rcvd += TmxData(kv.second).to_uint();
    }

    TmxData msgStats;
    msgStats[msg.get_source()]["Sent"] = sent;
    msgStats[msg.get_source()]["Received"] = rcvd;
    this->set_status("RSU Messages", msgStats.get_container());
}

template <>
void TmxPlugin::on_message_received<TmxData const, v2x::RSU::handle_ifm_mib>(TmxData const &data,
                                                                             TmxMessage const &msg) {
    static std::mutex _singleExec;
    std::lock_guard<std::mutex> lock(_singleExec);

    TLOG(DEBUG) << std::this_thread::get_id() << ": Enter " << TMX_PRETTY_FUNCTION << " with " << msg.to_string();

    auto plugin = dynamic_cast<v2x::RSU::RSUImmediateForwardPlugin *>(this);
    if (!plugin)
        return;

    typedef typename std::remove_pointer<decltype(plugin)>::type plugin_type;

    // Make sure the SNMP source is set
    if (msg.get_source().empty())
        return;

    // Try to pull the IFM table
    int row = 0;
    if (data && data.is_map()) {
        TLOG(DEBUG) << "Received IFM data: " << data;

        v2x::RSU::_cfgThrottle.Touch(0);

        // Make sure there is good data to check
        const auto &map = data.to_map();

        auto RSUs = this->get_config("RSUs", &(plugin->_dataLock));

        for (auto const &kv: map) {
            const TmxData value{ kv.second };
            if (!row && value.is_empty())
                break;

            // Get the row number
            auto idx = kv.first.find_last_of('.');
            if (idx < kv.first.length())
                row = TmxData(kv.first.substr(idx + 1)).to_uint();

            if (row) {
                bool transmit = true;
                auto nm = kv.first.substr(strlen("RSU-MIB::rsuIFM"));
                if (std::strncmp("DsrcMsgId", nm.c_str(), 9) == 0) {
                    RSUs[msg.get_source()][row - 1]["ID"] = kv.second;

                    // Look up the type
                    std::string msgType = TmxData(kv.second);
                    auto topic = v2x::RSU::get_message_topic_name(msgType);
                    if (!topic.empty())
                        RSUs[msg.get_source()][row - 1]["Type"] = topic;
                }
                if (std::strncmp("Psid", nm.c_str(), 4) == 0) {
                    auto id = TmxData(kv.second).to_string();
                    if (id.substr(0, 1) == "0")
                        id = id.substr(0, 2);
                    else if (id.substr(0, 1) == "8")
                        id = id.substr(0, 4);
                    else if (id.substr(0, 1) == "C")
                        id = id.substr(0, 6);

                    RSUs[msg.get_source()][row - 1]["PSID"] = "0x" + id;
                }
                if (std::strncmp("TxChannel", nm.c_str(), 9) == 0)
                    RSUs[msg.get_source()][row - 1]["Channel"] = kv.second;
                if (std::strncmp("Enable", nm.c_str(), 6) == 0) {
                    transmit = transmit && (TmxData(kv.second).to_string() == std::string("on(1)"));
                    RSUs[msg.get_source()][row - 1]["Transmit"] = transmit;
                }
                if (std::strncmp("Status", nm.c_str(), 6) == 0) {
                    transmit = transmit && (TmxData(kv.second).to_string() == std::string("active(1)"));
                    RSUs[msg.get_source()][row - 1]["Transmit"] = transmit;
                }
            }

        }

        if (row)
            this->set_config("RSUs", RSUs.get_container(), &(plugin->_dataLock));
    }

    if (!data || row) {
        // Get the next row of the IFM table
        v2x::RSU::next_request(v2x::RSU::handle_ifm_mib{ }, this, row + 1);
    }
}

namespace v2x {
namespace RSU {

void RSUImmediateForwardPlugin::init() noexcept {
    // The config parameter handler
    this->register_handler<TmxPluginDataUpdate>(this->get_topic("config/RSUs"), (TmxPlugin *) this,
                                                &TmxPlugin::on_message_received<TmxPluginDataUpdate const>);

    // Register the SNMP message handlers
    this->register_handler<handle_rsu_mib>(handle_rsu_mib::topic, (TmxPlugin *) this,
                                           &TmxPlugin::on_message_received<TmxData const, handle_rsu_mib>);
    this->register_handler<handle_msg_stats>(handle_msg_stats::topic, (TmxPlugin *) this,
                                             &TmxPlugin::on_message_received<TmxData const, handle_msg_stats>);
    this->register_handler<handle_ifm_mib>(handle_ifm_mib::topic, (TmxPlugin *) this,
                                           &TmxPlugin::on_message_received<TmxData const, handle_ifm_mib>);

    // The assumption is that the channel configuration is set up so that only the
    // RSU resource will automatically register to listen for the above SNMP topics
    TmxPlugin::init();

    // This handler is internal use only
    this->register_handler<std::true_type>("check_status", (TmxPlugin *) this,
                                           &TmxPlugin::on_message_received<TmxData const, std::true_type>);


    // Update the SNMP information at least every second
    _updateThrottle.set_Frequency(std::chrono::seconds(1));

    // Update the IFM table at least every 30 seconds
    _cfgThrottle.set_Frequency(_updateThrottle.get_Frequency() * 30);
}

} /* End namespace RSU */
} /* End namespace v2x */
} /* End namespace plugin */
} /* End namespace tmx */
