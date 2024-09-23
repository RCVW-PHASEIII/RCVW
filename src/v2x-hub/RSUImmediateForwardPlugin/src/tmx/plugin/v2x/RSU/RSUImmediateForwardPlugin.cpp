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

#include <tmx/broker/TmxBrokerContext.hpp>
#include <tmx/broker/TmxBrokerClient.hpp>
#include <tmx/common/TmxLogger.hpp>
#include <tmx/message/codec/serializer/TmxDataSerializer.hpp>
#include <tmx/plugin/TmxChannel.hpp>
#include <tmx/plugin/utils/FrequencyThrottle.hpp>

#include <algorithm>
#include <iostream>
#include <chrono>
#include <sstream>
#include <thread>
#include <boost/format.hpp>

#define TMX_DEFAULT_IFM_PORT 1516
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

static constexpr auto Key_SkippedNoDsrcMetadata = "Messages Skipped (No DSRC metadata)";
static constexpr auto Key_SkippedNoMessageRoute = "Messages Skipped (No route)";
static constexpr auto Key_SkippedInvalidUdpClient = "Messages Skipped (Invalid UDP Client)";

RSUImmediateForwardPlugin::RSUImmediateForwardPlugin() {
    // Private tags for the handlers
    struct on_config_update { };

    this->_muteRadio = false;
    this->_statusThrottle.set_Frequency(std::chrono::seconds(1));

    this->register_handler<on_config_update>(this->get_topic("config/MuteRadio"), this,
                                             &RSUImmediateForwardPlugin::handle_config_update);
}

TmxTypeDescriptor RSUImmediateForwardPlugin::get_descriptor() const noexcept {
    auto _descr = TmxPlugin::get_descriptor();
    return { _descr.get_instance(), typeid(*this), type_fqname(*this).data() };
}

void RSUImmediateForwardPlugin::on_message_received(tmx::message::TmxMessage const &msg) {
    static const types::Any _empty{ };

    // If this message was sent through a J2375 topic, then relay un-decoded to the handler
    TmxTypeRegistry _regCheck{ msg.get_topic().data() };
    if (_regCheck.get_parent().get_namespace() == "J2735") {
        this->invoke_handlers(_empty, msg, msg.get_topic());
        return;
    }

    // Otherwise, process regularly
    TmxPlugin::on_message_received(msg);
}

void RSUImmediateForwardPlugin::handle_config_update(TmxPluginDataUpdate const &data, message::TmxMessage const &) {
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

    if (strcmp("MuteRadio", str.c_str()) == 0) {
        this->_muteRadio = data.get_value().to_bool();
    }
}

void RSUImmediateForwardPlugin::handle_incoming(types::Any const &, TmxMessage const &msg) {
    TLOG(DEBUG1) << "Received: " << msg.get_container();

    if (_muteRadio)
        return;

    this->_recvMsgs++;

    // The first two bytes of the payload indicates the message ID
    std::string idStr{ msg.get_payload_string().c_str(), 4 };
    std::uint16_t id = std::strtoul(idStr.c_str(), nullptr, 16);
    TLOG(DEBUG) << "Message ID 0x" << idStr << " (" << id << ") received from topic " << msg.get_topic();

    bool foundMessageType = false;

    // Loop through each RSU destination
    TmxData const &RSUs = this->get_config("RSUs", &(this->_dataLock));

    for (auto rsu: RSUs.to_map()) {
        TmxData const ifmTable { rsu.second };

        TLOG(DEBUG) << "IFM table for " << rsu.first << ": " << ifmTable.to_string();

        for (std::size_t i = 0; i < (std::size_t)ifmTable; i++) {
            auto const &rsuInfo = ifmTable[i];

            // Self-mute if there are no message types defined yet for the RSU
            if (!rsuInfo["Type"])
                continue;

            if (id != rsuInfo["ID"].to_uint())
                continue;

            std::string psid;
            std::string channel;
            if (rsuInfo["PSID"]) {
                psid = rsuInfo["PSID"].to_string();
            } else {
                char buf[11];
                int x = sprintf(buf, "0x%X", (unsigned int) ((msg.get_programmable_metadata() >> 16) & 0xFF));
                buf[x] = '\0';

                psid.assign(buf);
            }

            if (rsuInfo["Channel"])
                channel = rsuInfo["Channel"].to_string();
            else
                channel = std::to_string((unsigned int) ((msg.get_programmable_metadata() >> 8) & 0xFF));

            stringstream os;

            os << "Version=0.7" << "\n";
            os << "Type=" << rsuInfo["Type"].to_string() << "\n";
            os << "PSID=" << psid << "\n";
            os << "Priority=7" << "\n" << "TxMode=CONT" << "\n";
            os << "TxChannel=" << channel << "\n";
            os << "TxInterval=0" << "\n" << "DeliveryStart=\n" << "DeliveryStop=\n";
            os << "Signature=" << TmxData(this->get_config("Signature")).to_string() << "\n";
            os << "Encryption=False\n";
            os << "Payload=" << msg.get_payload_string() << "\n";

            message::TmxMessage ifmMsg;
            ifmMsg.set_encoding("string");
            ifmMsg.set_timestamp(msg.get_timestamp());
            ifmMsg.set_source(msg.get_source());
            ifmMsg.set_topic("RSU/IFM");
            ifmMsg.set_payload(os.str());

            this->broadcast(ifmMsg);
            this->_sentMsgs++;
        }
    }
}

TmxError RSUImmediateForwardPlugin::main() noexcept {
    // Add an empty destination list to start
    if (!this->get_config("RSUs", &(this->_dataLock)))
        this->set_config("RSUs", types::make_any(types::Null()), &(this->_dataLock));

    TmxMessage msg;
    msg.set_topic("check_status");

    TLOG(DEBUG) << "Main thread is " << std::this_thread::get_id();

    while (this->is_running()) {
        std::this_thread::sleep_for(this->_statusThrottle.get_Frequency<std::chrono::milliseconds>() / 10);

        // Trigger status checks
        msg.set_timepoint();
        this->invoke_handlers(types::Null(), msg);

        if (this->_statusThrottle.Monitor(0)) {
            this->set_status("ReceivedMessages", (std::int64_t) this->_recvMsgs);
            this->set_status("SentMessages", (std::int64_t) this->_sentMsgs);
        }
    }

    return { };
}

types::Array<types::Any> RSUImmediateForwardPlugin::get_config_description() const noexcept {
    TmxData _data;
    _data[0]["key"] = std::string("Signature");
    _data[0]["default"] = false;
    _data[0]["description"] = std::string("Sign the message before sending to the RSU?");
    _data[1]["key"] = std::string("MuteRadio");
    _data[1]["default"] = false;
    _data[1]["description"] = std::string("Mute sending messages to the radio");

    return _data.to_array();
}

} /* End namespace RSU */
} /* End namespace v2x */
} /* End namespace plugin */
} /* End namespace tmx */

// The main entry point for this application.
int main(int argc, char *argv[])
{
    tmx::plugin::v2x::RSU::RSUImmediateForwardPlugin _plugin;
    return tmx::plugin::run(_plugin, argc, argv);
}
