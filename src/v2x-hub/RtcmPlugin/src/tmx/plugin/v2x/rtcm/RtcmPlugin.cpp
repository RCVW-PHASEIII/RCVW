/*
 * RtcmPlugin.cpp
 *
 *  Created on: Mar 22, 2018
 *      Author: gmb
 */

#include <tmx/plugin/v2x/rtcm/RtcmPlugin.hpp>

#include <tmx/broker/TmxBrokerClient.hpp>
#include <tmx/common/TmxLogger.hpp>
#include <tmx/message/TmxData.hpp>
#include <tmx/message/j2735/202007/MessageFrame.h>
#include <tmx/message/v2x/rtcm/RtcmMessage.hpp>
#include <tmx/plugin/utils/Clock.hpp>
#include <tmx/plugin/utils/FrequencyThrottle.hpp>

#include <chrono>
#include <cstring>
#include <mutex>
#include <regex>
#include <sstream>
#include <thread>

using namespace tmx::common;

namespace tmx {
namespace plugin {
namespace v2x {
namespace rtcm {

// Private structures for the handlers
struct GGA {};
struct J2735 {};

class DummyNtripBroker: public broker::TmxBrokerClient {
public:
    DummyNtripBroker() noexcept {
        this->register_broker("ntrip");
    }

    TmxTypeDescriptor get_descriptor() const noexcept override {
        // Return the HTTP broker
        broker::TmxBrokerContext _tmp { "http://localhost" };
        auto broker = broker::TmxBrokerClient::get_broker(_tmp);
        if (broker)
            return broker->get_descriptor();
        else
            return TmxBrokerClient::get_descriptor();
    }
};

RtcmPlugin::RtcmPlugin() {
    static DummyNtripBroker _unused;

    message::v2x::rtcm::register_rtcm();

    // (1) Listens for a GGA string to probe an NTRIP caster with
    //      (a) From a GPSD broker
    this->register_handler<GGA>("nmea/GGA", this, &RtcmPlugin::on_gga_received);
    //      (b) From another V2X Hub plugin
    this->register_handler<GGA>("V2X/Location/GGA", this, &RtcmPlugin::on_gga_received);
    //      (c) From the configuration of this plugin
    this->register_handler<GGA>(this->get_topic("config/GGA"), this, &RtcmPlugin::on_gga_received);

#ifndef IGNORE_RTCM3
    // (2) Listens for incoming RTCM3 Messages
    //      (a) From a GPSD broker
    this->register_handler<message::v2x::rtcm::RTCM3Message>("gpsd/RTCM3", this, &RtcmPlugin::on_rtcm_received);
    //      (b) From another V2X Hub source
    this->register_handler<message::v2x::rtcm::RTCM3Message>("V2X/RTCM3", this, &RtcmPlugin::on_rtcmmsg_received);
#endif
#ifndef IGNORE_RTCM2
    // (3) Listens for incoming RTCM2 Messages
    //      (a) From a GPSD broker
    this->register_handler<message::v2x::rtcm::RTCM2Message>("gpsd/RTCM2", this, &RtcmPlugin::on_rtcm_received);
    //      (b) From another V2X Hub plugin
    this->register_handler<message::v2x::rtcm::RTCM2Message>("V2X/RTCM2", this, &RtcmPlugin::on_rtcmmsg_received);
#endif
}

TmxTypeDescriptor RtcmPlugin::get_descriptor() const noexcept {
    auto _descr = TmxPlugin::get_descriptor();
    return { _descr.get_instance(), typeid(*this), type_fqname(*this).data() };
}

common::types::Array<common::types::Any> RtcmPlugin::get_config_description() const noexcept {
    message::TmxData _data;
    _data[0]["key"] = "GGA";
    _data[0]["description"] = "Set the position bootstrap for NTRIP with the given NMEA GGA sentence.";

    return _data.to_array();
}

void RtcmPlugin::init() noexcept {
    TmxPlugin::init();

    // Check the channel configuration for NTRIP
    for (auto &channel: this->get_channels()) {
        if (std::strcmp("ntrip", channel->get_context().get_scheme().c_str()))
            continue;

        std::lock_guard<std::mutex> lock(this->_lock);
        _ntrip = channel->get_context().get_id();
    }
}

void RtcmPlugin::on_gga_received(types::Any const &, message::TmxMessage const &msg) {
    TLOG(DEBUG3) << "Enter " << TMX_PRETTY_FUNCTION << " with " << msg.get_payload_string();

    std::lock_guard<std::mutex> lock(this->_lock);
    this->_gga = msg.get_payload_string();
}

#ifndef IGNORE_RTCM3
void RtcmPlugin::on_rtcmmsg_received(message::v2x::rtcm::RTCM3Message const &rtcm3Msg, message::TmxMessage const &msg) {
    TLOG(DEBUG3) << "Enter " << TMX_PRETTY_FUNCTION << " with " << msg.to_string();

    if (!rtcm3Msg.is_valid())
        return;

    auto msgType = enums::enum_cast<message::v2x::rtcm::RTCM3_MESSAGE_TYPE>(rtcm3Msg.get_MessageNumber());

    TLOG(DEBUG1) << "Received RTCM3 "
                 << (msgType.has_value() ? enums::enum_name(msgType.value()) : std::to_string(rtcm3Msg.get_MessageNumber()))
                 << " message of " << rtcm3Msg.get_MessageLength() << " bytes.";

    // See if the topic is from the NTRIP context
    std::shared_ptr<TmxChannel> ntrip;
    {
        std::lock_guard<std::mutex> lock(this->_lock);
        ntrip = this->get_channel(this->_ntrip);
    }

    MessageFrame frame;

    auto &rtcm = frame.value.choice.RTCMcorrections;
    const_string nm = type_short_name(rtcm);
    frame.messageId = 0x1C;
    frame.value.present = MessageFrame__value_PR_RTCMcorrections;

    rtcm.anchorPoint = nullptr;
    rtcm.regional = nullptr;
    rtcm.rtcmHeader = nullptr;

    if (this->_count > 127)
        this->_count = 0;

    rtcm.msgCnt = this->_count++;
    rtcm.rev = RTCM_Revision_rtcmRev3;

    // Get the bytes
    auto msgBytes = byte_string_decode(msg.get_payload_string());

    // TODO: Support JSON encoded RTCM message
    typename std::chrono::system_clock::duration td { msg.get_timestamp() };
    typename std::chrono::system_clock::time_point tp(td);
    auto sec = std::chrono::duration_cast<std::chrono::seconds>(tp.time_since_epoch()).count();
    auto *utctime = std::gmtime(&sec);
    MinuteOfTheYear_t ts = utctime->tm_min +
                            ((utctime->tm_hour + utctime->tm_isdst) * 60) +
                             (utctime->tm_yday * 24 * 60);
    rtcm.timeStamp = (MinuteOfTheYear_t *)&ts;

    RTCMmessage_t rtcmMsg;
    rtcmMsg.size = msgBytes.length();
    rtcmMsg.buf = (std::uint8_t *)calloc(rtcmMsg.size, sizeof(std::uint8_t));
    memcpy(rtcmMsg.buf, msgBytes.data(), rtcmMsg.size);

    RTCMmessage_t *msgs = &rtcmMsg;
    rtcm.msgs.list.array = &msgs;
    rtcm.msgs.list.count = 1;

//    xer_fprint(stdout, &asn_DEF_MessageFrame, &frame);

    char buffer[4096];
    auto result = uper_encode_to_buffer(&asn_DEF_MessageFrame, nullptr, &frame, buffer, 4096);

    free(rtcmMsg.buf);
    rtcmMsg.buf = nullptr;

    if (result.encoded < 0) {
        TLOG(ERR) << "Failed on " << (result.failed_type ? result.failed_type->name : " unknown type");
        this->broadcast<TmxError>({ 1, "Unable to encode " + std::string(nm) }, this->get_topic("error"),
                                  __FUNCTION__);
    } else {
        // For UPER encoding, the number of bytes encoded must be adjusted
        result.encoded = (result.encoded + 7) / 8;

        message::TmxMessage j2735Msg;
        j2735Msg.set_id(nm.data());
        j2735Msg.set_topic("J2735/RTCM");
        j2735Msg.set_source(msg.get_source());
        j2735Msg.set_timepoint();
        j2735Msg.set_payload(byte_string_encode(to_byte_sequence(buffer, result.encoded)));
        j2735Msg.set_encoding("asn.1-uper");

        this->broadcast(j2735Msg);
    }
}
#endif

void RtcmPlugin::on_rtcm_received(message::TmxData const &data, message::TmxMessage const &msg) {
    TLOG(DEBUG3) << "Enter " << TMX_PRETTY_FUNCTION << " with " << msg.to_string();

    message::codec::TmxCodec codec { msg };

#ifndef IGNORE_RTCM3
    auto byteStr = codec.get_payload_bytes();

    // If this comes from GPSd, it will be a JSON string
    if (std::strncmp("gpsd/RTCM3", msg.get_topic().c_str(), 10) == 0) {
        std::size_t dataSz = data["length"];

        if (1005 == (int)data["type"]) {
            auto sys = data["system"].to_array();
            types::UInt1 gps;
            types::UInt1 glon;
            types::UInt1 gal;
            types::UInt1 refS = data["refstation"] ? 1 : 0;
            types::UInt1 sro = data["src"] ? 1 : 0;

            for (auto &val: sys) {
                auto const &str = message::TmxData(val).to_string();
                if (std::strcmp("GPS", str.c_str()) == 0)
                    gps = 1;
                if (std::strcmp("GLONASS", str.c_str()) == 0)
                    glon = 1;
                if (std::strcmp("GALILEO", str.c_str()) == 0)
                    gal = 1;
            }

            types::Int38 X = (double) data["x"] * 10000;
            types::Int38 Y = (double) data["y"] * 10000;
            types::Int38 Z = (double) data["z"] * 10000;

            auto pt1 = types::pack(data["type"].to_uint<12>(), data["station_id"].to_uint<12>(), types::UInt6(), gps,
                                   glon);
            auto pt2 = types::pack(gal, refS, X);
            auto pt3 = types::pack(sro, types::UInt1(), Y);
            auto pt4 = types::pack(types::UInt2(), Z);

            byteStr.clear();
            for (auto &b: make_byte_cursor(*pt1))
                byteStr.push_back(b);

            std::size_t skip = ((pt2.numBytes * TMX_BITS_PER_BYTE) - pt2.numBits) / TMX_BITS_PER_BYTE;

            std::size_t i = 0;
            for (auto &b: make_byte_cursor(*pt2))
                if (i++ >= skip)
                    byteStr.push_back(b);

            i = 0;
            for (auto &b: make_byte_cursor(*pt3))
                if (i++ >= skip)
                    byteStr.push_back(b);

            i = 0;
            for (auto &b: make_byte_cursor(*pt4))
                if (i++ >= skip)
                    byteStr.push_back(b);
        } else if (!data["data"]) {
            // Ignore
            return;
        } else if (dataSz != (std::size_t) data["data"]) {
            this->broadcast<common::TmxError>({ EMSGSIZE, "Expecting " + std::to_string(dataSz) +
                                                " bytes, but only received " +
                                                std::to_string((std::size_t) data["data"]) },
                                              this->get_topic("error"), __FUNCTION__);
            return;
        } else {
            byteStr.clear();
            for (std::size_t i = 0; i < dataSz; i++)
                byteStr.push_back((byte_t) (std::strtoul(data["data"][i].to_string().c_str(), nullptr, 0) & 0xFF));
        }
    }

    TLOG(DEBUG1) << "Received payload: " << byteStr;

    std::string nm = message::v2x::rtcm::RtcmVersionName<message::v2x::rtcm::RTCM_VERSION::SC10403_3>();
    auto decoder = message::codec::TmxDecoder::get_decoder(nm);
    if (!decoder) {
        this->broadcast<TmxError>({ ENOTSUP, "Missing RTCM decoder " + nm }, this->get_topic("error"), __FUNCTION__);
        return;
    }

    message::v2x::rtcm::RTCM3Message rtcm3Msg;
    auto err = decoder->decode(rtcm3Msg, to_byte_sequence(byteStr.data(), byteStr.length()));
    if (err) {
        this->broadcast<TmxError>(err, this->get_topic("error"), __FUNCTION__);
        return;
    }

    err = codec.encode(rtcm3Msg, rtcm3Msg.get_version_name());
    if (err) {
        this->broadcast<TmxError>(err, this->get_topic("error"), __FUNCTION__);
        return;
    }

    codec.get_message().set_timestamp(msg.get_timestamp());
    codec.get_message().set_topic("V2X/RTCM3");
    this->broadcast(codec.get_message());
    this->invoke_handlers(rtcm3Msg, codec.get_message());
#endif
}

common::TmxError RtcmPlugin::main() noexcept {
    utils::FrequencyThrottle<std::string> ggaChange { std::chrono::seconds(1) };
    utils::FrequencyThrottle<std::string> ggaRefresh { std::chrono::seconds(10) };

    bool subscribed = false;

    while (this->is_running()) {
        std::string gga;
        std::string ntrip;
        {
            std::lock_guard<std::mutex> lock(this->_lock);
            gga = this->_gga;
            ntrip = this->_ntrip;
        }

        // No need to do anything without an NTRIP channel
        auto channel = this->get_channel(ntrip);
        if (!channel) {
            sleep(2);
            continue;
        }

        if (gga.empty()) {
            // Check for latitude and longitude and build our own GGA
            auto lat = this->get_config("latitude");
            auto lon = this->get_config("longitude");

            if (lat.is_empty() || lon.is_empty()) {
                sleep(2);
                continue;
            }

            double latDMS = std::abs((double)lat);
            latDMS = (100 * (int)latDMS) + (latDMS - (int)latDMS) * 60.0;

            double lonDMS = std::abs((double)lon);
            lonDMS = (100 * (int)lonDMS) + (lonDMS - (int)lonDMS) * 60.0;

            gga = "$";
            static auto fmtstr = "GPGGA,%09.2f,%08.3f,%c,%09.3f,%c,1,12,1.0,0.0,M,0.0,M,,";

            auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
            auto utc = std::gmtime(&now);

            char buf[128];
            int n = sprintf(buf, fmtstr, ((utc->tm_hour + utc->tm_isdst) * 10000 +
                                            utc->tm_min * 100 + utc->tm_sec * 1.0),
                            latDMS, (double)lat > 0 ? 'N' : 'S',
                            lonDMS, (double)lon > 0 ? 'E' : 'W');
            gga += to_char_sequence(buf, n);

            // Checksum is an XOR of all the character values in the string
            int chksum = 0;
            for (int i = 0; i < n; i++)
                chksum ^= buf[i];

            n = sprintf(buf, "*%2X", chksum);
            gga += to_char_sequence(buf, n);

            std::lock_guard<std::mutex> lock(this->_lock);
            if (this->_gga.empty())
                this->_gga = gga;
        }

        // If the NTRIP broker is not connected, set the GGA as a header and subscribe
        if (!subscribed) {
            message::TmxData params{ channel->get_context().get_parameters() };
            params["additional-headers"]["Ntrip-GGA"] = std::string(gga);
            params["additional-headers"]["Ntrip-Version"] = std::string("Ntrip/2.0");
            params["message-break"] = std::string("\r\n");

            auto &topic = std::filesystem::path(channel->get_context().get_path()).filename().native();

            this->register_handler<J2735>(topic, this, &RtcmPlugin::on_rtcm_received);
            channel->read_messages(topic);
            channel->get_context().get_receive_sem().wait_for(channel->get_context().get_receive_lock(),
                    std::chrono::seconds(10), [channel]() -> bool {
                        auto broker = broker::TmxBrokerClient::get_broker(channel->get_context());
                        if (broker)
                            return broker->is_connected(channel->get_context());

                        return false;
                    });

            subscribed = true;
            sleep(2);
            continue;
        }

        // Otherwise, make sure the connection is still connected
        if (channel->get_context().get_state() < broker::TmxBrokerState::connected) {
            subscribed = false;
            sleep(2);
            continue;
        }

        // Check the throttles
        if (!ggaChange.Monitor(gga)) {
            sleep(2);
            continue;
        }

        message::TmxMessage ggaMsg;
        ggaMsg.set_timepoint();
        ggaMsg.set_topic("ntrip/GGA");
        ggaMsg.set_encoding("json");
        ggaMsg.set_payload(gga + "\r\n\r\n");

        channel->write_message(ggaMsg);
    }

    return { };
}

} /* namespace rtcm */
} /* namespace v2x */
} /* namespace plugin */
} /* namespace tmx */

#include <tmx/message/codec/asn/TmxAsnDot1Schema.hpp>

int main(int argc, char *argv[]) {
    tmx::plugin::v2x::rtcm::RtcmPlugin _plugin;
	return run(_plugin, argc, argv);
}
