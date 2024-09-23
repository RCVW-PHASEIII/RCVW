/*
 * DifferentialGPSPlugin.cpp
 *
 *  Created on: Jul 20, 2018
 *      @author: Gregory M. Baumgardner
 *  Edited on: May 30, 2024
 *      @author: Noah Limes
 */

#include <atomic>
#include <fstream>
#include <mutex>

#include <DifferentialGPSPlugin.hpp>
#include <DifferentialGPS_Configuration.hpp>

#include <tmx/plugin/TmxChannel.hpp>
#include <tmx/plugin/utils/Clock.hpp>

#include <tmx/broker/TmxBrokerContext.hpp>
#include <tmx/broker/TmxBrokerClient.hpp>

#include <tmx/common/TmxLogger.hpp>

#include <tmx/message/j2735/202007/MessageFrame.h>
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
namespace differentialGPS {

/**
* Default Constructor. Good Place to initialize atomic variables.
*
* @param name string used to identify the plugin.
*/
DifferentialGPSPlugin::DifferentialGPSPlugin() {
    //Set Throttles
    this->_statusThrottle.set_Frequency(std::chrono::seconds(1));

    //Register handlers
    this->register_handler<on_location>("gpsd/TPV" , this, &DifferentialGPSPlugin::handle_location);
    this->register_handler<on_rtcm>("J2735/RTCM", this, &DifferentialGPSPlugin::handle_rtcm);
}

DifferentialGPSPlugin::~DifferentialGPSPlugin() = default;

TmxTypeDescriptor DifferentialGPSPlugin::get_descriptor() const noexcept {
    auto _descr = TmxPlugin::get_descriptor();
    return { _descr.get_instance(), typeid(*this), common::type_fqname(*this).data() };
}

types::Array<types::Any> DifferentialGPSPlugin::get_config_description() const noexcept {
    message::TmxData _data;
    auto decoder = message::codec::TmxDecoder::get_decoder("json");
    if (decoder)
        decoder->decode(_data.get_container(), to_byte_sequence(DIFFERENTIAL_GPS_CONFIGURATION_JSON));
    return _data.to_array();
}

void DifferentialGPSPlugin::set_config(common::const_string key, const types::Any &val, std::mutex *mtx) {
    if (!mtx)
        mtx = &this->_cfgLock;

    TmxPlugin::set_config(key, val, mtx);
}

void DifferentialGPSPlugin::handle_rtcm(message::TmxData const &data, message::TmxMessage const &msg) {
    TLOG(DEBUG3) << "Received RTCM message " << msg.get_payload_string();

    TmxData device;
    TmxData version;

    {
        lock_guard<mutex> lock(_cfgLock);
        device = this->get_config("device");
        version = this->get_config("rtcm-version");
    }

    if (device.is_empty())
        return;

    if (!_doWrite)
        return;

    // Decode the MessageFrame
    auto bytes = byte_string_decode(msg.get_payload_string());
    MessageFrame *frame = nullptr;
    auto ret = uper_decode_complete(nullptr, &asn_DEF_MessageFrame, (void **)&frame, bytes.data(), bytes.length());
    if (ret.code != RC_OK) {
        ASN_STRUCT_FREE(asn_DEF_MessageFrame, frame);

        this->broadcast<TmxError>(
                { ret.code, "Decoding failed after " + std::to_string(ret.consumed) + " bytes" },
                this->get_topic("error"), __FUNCTION__);
        return;
    }

    string err = "";
    if (frame && frame->value.present == MessageFrame__value_PR_RTCMcorrections) {
        TLOG(DEBUG3) << "Decoded " << ret.consumed << " bytes.";

        lock_guard<mutex> lock(_writeLock);
        ofstream out;
        out.open(device.to_string().c_str(), ios::binary | ios::out);

        for (std::size_t i = 0; i < frame->value.choice.RTCMcorrections.msgs.list.count; i++) {
            auto ptr = frame->value.choice.RTCMcorrections.msgs.list.array[i];
            if (ptr) {
                auto str = to_char_sequence(ptr->buf, ptr->size);
                TLOG(DEBUG2) << "Writing " << str.length() << " bytes to " << device;

                out.write(str.data(), str.length());
                if (out.fail()) {
                    this->broadcast<TmxError>(
                            { errno, "Failed to write RTCM " + byte_string_encode(to_byte_sequence(str))
                                     + " message to device: " + std::strerror(errno) },
                            this->get_topic("error"), __FUNCTION__);
                } else {
                    _msgCount++;
                    _byteCount += bytes.size();
                }
            }
        }

        out.close();
    }

    ASN_STRUCT_FREE(asn_DEF_MessageFrame, frame);

    if (_statusThrottle.Monitor(0)) {
        this->set_status("RTCM Message Written", (uint64_t) _msgCount);
        this->set_status("RTCM Bytes Written", (uint64_t) _byteCount);
        this->set_status("Error", err);
    }
}

void DifferentialGPSPlugin::handle_location(TmxData const &data, tmx::message::TmxMessage const &msg) {

    TLOG(DEBUG3) << "Received Location message " << data;

     auto fix = enums::enum_cast<message::v2x::FixTypes>(data["mode"]);
     if (fix.has_value())
         _doWrite = (fix.value() >= message::v2x::FixTypes::ThreeD);
}

void DifferentialGPSPlugin::on_message_received(message::TmxMessage const &msg) {
    // Skip decoding for the J2735 RTCM message
    if (msg.get_topic() == "J2735/RTCM")
        this->handle_rtcm({ }, msg);
    else
        TmxPlugin::on_message_received(msg);
}

} /* End namespace tmx */
} /* End namespace plugin */
} /* End namespace rcvw */
} /* End namespace DifferentialGPSPlugin */

int main(int argc, char* argv[])
{
    tmx::plugin::v2x::differentialGPS::DifferentialGPSPlugin _plugin;
    return run(_plugin, argc, argv);
}
