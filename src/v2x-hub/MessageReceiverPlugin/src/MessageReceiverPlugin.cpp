/*
 * MessageReceiver.cpp
 *
 *  Created on: May 10, 2016
 *      Author: ivp
 *  Edited: May 16, 2024
 *      @author: Noah Limes
 */

//messages from radio only bytes figure out which j

#include "MessageReceiverPlugin.hpp"
#include "MessageReceiver_Configuration.hpp"

#include <tmx/common/TmxLogger.hpp>
#include <tmx/message/codec/TmxCodec.hpp>
#include <tmx/message/codec/serializer/TmxDataSerializer.hpp>
#include <tmx/plugin/utils/Clock.hpp>

#include <mutex>
#include <regex>
#include <stdexcept>
#include <thread>

// Message helpers
#include <tmx/message/v2x/LocationMessageEnumTypes.hpp>
#include <tmx/message/v2x/VehicleBasicMessage.hpp>

#include <MessageFrame.h>

#define ABBR_BSM 1000
#define ABBR_SRM 2000
#define ABBR_VBM 3000

using namespace std;
using namespace boost::asio;
using namespace tmx::common;
using namespace tmx::common::types;
using namespace tmx::message;
using namespace tmx::plugin::utils;
using namespace tmx::message::codec::serializer;

// BSMs may be 10 times a second, so only send errors at most every 2 minutes
#define ERROR_WAIT_MS 120
#define STATUS_WAIT_MS 1

namespace tmx {
namespace plugin {
namespace v2x {
namespace MessageReceiver {

//convert bytes to tmx message based off its id, look up in registry to get its TmxTypeDescriptor
static TmxTypeRegistry j2735_reg { "tmx.message.J2735" };

static utils::FrequencyThrottle<std::string> _errThrottle;

DSRCmsgID_t get_message_id(const_string msgType) {
    auto descr = j2735_reg.get("get-message-id");
    auto fn = descr.as_instance<std::function<long(std::string)> >();
    if (fn)
        return (*fn)(std::string(msgType.data(), msgType.length()));
    else
        return 0;
}

std::string get_message_topic_name(const_string msgType) {
    auto descr = j2735_reg.get("get-message-name");
    auto fn = descr.as_instance<std::function<std::string(std::string)> >();
    if (fn)
        return (*fn)(std::string(msgType.data(), msgType.length()));
    else
        return "";
}

std::string get_message_type_name(const_string msgType) {
    auto descr = j2735_reg.get("get-type-name");
    auto fn = descr.as_instance<std::function<std::string(std::string)> >();
    if (fn)
        return (*fn)(std::string(msgType.data(), msgType.length()));
    else
        return "";
}

asn_TYPE_descriptor_t *get_type_descriptor(const_string msgType) {
    auto descr = j2735_reg.get("get-type-descriptor");
    auto fn = descr.as_instance<std::function<void *(std::string)> >();
    if (fn)
        return static_cast<asn_TYPE_descriptor_t *>((*fn)(std::string(msgType.data(), msgType.length())));
    else
        return nullptr;
}

// Handler tags
struct incoming { };
struct j2735 { };

// Simulator messages
typedef std::integral_constant<std::uint16_t, 1000> simBSM;
typedef std::integral_constant<std::uint16_t, 2000> simSRM;
typedef std::integral_constant<std::uint16_t, 3000> simVBM;

} /* End namespace MessageReceiver */
} /* End namespace v2x */

template <>
void TmxPlugin::on_message_received<TmxData const, v2x::MessageReceiver::incoming>(TmxData const &,
                                                                                   TmxMessage const &msg) {
    TLOG(DEBUG3) << "Enter " << TMX_PRETTY_FUNCTION << " with " << msg.to_string();

    if (!msg.get_length())
        return;

    // This should have set the source of the data as the channel
    auto channel = this->get_channel(msg.get_source());
    if (!channel)
        return;

    auto channelCallback = TmxTypeRegistry("tmx.plugin.channels").get("TmxChannelOnMessageReceived");

    codec::TmxCodec codec { msg };
    auto payloadBytes = byte_string_decode(codec.get_payload_string());

    UInt<16> msgId;
    UInt<16> msgVersion;
    UInt<16> id;
    UInt<16> len;

    typedef bit_count<decltype(msgId), decltype(msgVersion), decltype(id), decltype(len)> count;
    typedef UInt<count::value> header_t;

    if (payloadBytes.length() < header_t::numBytes) {
        // At least get the message ID
        msgId = get_value<typename decltype(msgId)::value_type>(payloadBytes.substr(0, decltype(msgId)::numBytes));
    } else {
        auto header = get_value<typename header_t::value_type>(payloadBytes.substr(0, header_t::numBytes));
        unpack(header, msgId, msgVersion, id, len);
    }

    TLOG(DEBUG1) << "Received messageId: " << msgId << ", msgVersion: " << msgVersion
                 << ", id: " << id << ", len: " << len;

    // Update things for the status messages
    auto plugin = dynamic_cast<v2x::MessageReceiver::MessageReceiverPlugin *>(this);
    if (plugin)
        plugin->_totalBytes += payloadBytes.length();

    types::Any const _id { channel->get_context().get_id() };

    // Extract the first two bytes, combine the two bytes into an integer (assuming big-endian order)
    if (v2x::MessageReceiver::simBSM::value == msgId) {
        codec.get_message().set_topic("Simulated/BSM");
        codec.get_message().set_payload(byte_string_encode(payloadBytes.substr(header_t::numBytes, len)));
    } else if (v2x::MessageReceiver::simSRM::value == msgId) {
        codec.get_message().set_topic("Simulated/SRM");
        codec.get_message().set_payload(byte_string_encode(payloadBytes.substr(header_t::numBytes, len)));
    } else if (v2x::MessageReceiver::simVBM::value == msgId) {
        codec.get_message().set_topic("Simulated/VBM");
        codec.get_message().set_payload(byte_string_encode(payloadBytes.substr(header_t::numBytes, len)));
    } else if (msgId > 0) {
        // Assume it is a J2735 message
        codec.get_message().set_topic("J2735/UNKNOWN");
    }

    auto err = common::dispatch(channelCallback, _id, codec.get_message());
    if (err)
        this->broadcast<TmxError>(err, this->get_topic("error"), __FUNCTION__);
}

template <>
void TmxPlugin::on_message_received<types::String8 const, v2x::MessageReceiver::j2735>(types::String8 const &,
                                                                                       TmxMessage const &msg) {
    TLOG(DEBUG3) << "Enter " << TMX_PRETTY_FUNCTION << " with " << msg.to_string();

    if (!this->get_config("enable-j2735"))
        return;

    UInt<16> msgId;

    auto payloadBytes = byte_string_decode(msg.get_payload_string());

    // Try up to the 10 times to find a message ID, in case there is a header
    DSRCmsgID_t id = 0;
    for (std::size_t i = 0; i < 10 && id <= 0; i++) {
        // Skip ahead to a leading zero byte
        // V2X message IDs are 0x00-0xFF, but the encoding of the frame uses 2 bytes (0x0000-0x00FF)
        while (payloadBytes.length() > 1 && (std::uint8_t)payloadBytes.front())
            payloadBytes = payloadBytes.substr(1);

        if (payloadBytes.length() > decltype(msgId)::numBytes)   // Make sure this is a valid J2735 type
            msgId = get_value<typename decltype(msgId)::value_type>(payloadBytes.substr(0, decltype(msgId)::numBytes));

        if (msgId > 0)
            id = v2x::MessageReceiver::get_message_id(std::to_string(msgId));
    }

    if (id > 0) {
        auto msgType = std::to_string(id);

        // Compose the message
        TmxMessage fwdMsg { msg };

        auto topic = v2x::MessageReceiver::get_message_topic_name(msgType);
        fwdMsg.set_id(v2x::MessageReceiver::get_message_type_name(msgType));
        fwdMsg.set_topic("J2735/" + topic);
        fwdMsg.set_payload(byte_string_encode(payloadBytes));
        fwdMsg.set_encoding("asn.1-uper");

        this->broadcast(fwdMsg);

        // Increment the totalCount map (thread-safe)
        auto plugin = dynamic_cast<v2x::MessageReceiver::MessageReceiverPlugin *>(this);
        if (plugin) {
            std::lock_guard<std::mutex> lock(plugin->_dataLock);
            // Initialize the atomic count for this message type if not already present
            if (!plugin->_totalCount.count(topic))
                plugin->_totalCount[topic] = 0;

            plugin->_totalCount[topic]++;
        }
    } else {
        if (v2x::MessageReceiver::_errThrottle.Monitor(std::to_string(*msgId)))
            this->broadcast<TmxError>({ EINVAL, "Request for invalid J2735 message " + msg.get_payload_string() },
                                      this->get_topic("error"), __FUNCTION__);
    }
}

template <>
void TmxPlugin::on_message_received<types::String8 const, v2x::MessageReceiver::simBSM>(types::String8 const &,
                                                                                        TmxMessage const &msg) {
    TLOG(DEBUG3) << "Enter " << TMX_PRETTY_FUNCTION << " with " << msg.to_string();

    codec::TmxCodec codec { msg };
    auto bytes = byte_string_decode(msg.get_payload_string());

    //extract data
    //vehicleId(4), heading*M(4), speed*K(4), (latitude+180)*M(4), (longitude+180)*M(4), elevation (4)
    typedef UInt<32> vehicleId;
    typedef UInt<32> heading;
    typedef UInt<32> speed;
    typedef UInt<32> lat;
    typedef UInt<32> lon;
    typedef UInt<32> alt;

    typedef bit_count<vehicleId, heading, speed, lat, lon, alt> count;

    if (bytes.length() != count::value / TMX_BITS_PER_BYTE)
        return;

    auto vehId = get_value<vehicleId::value_type>(bytes.substr(0, vehicleId::numBytes));
    bytes = bytes.substr(vehicleId::numBytes);

    auto now = std::chrono::system_clock::now();
    if (msg.get_timestamp() > 0) {
        std::chrono::system_clock::duration dur { msg.get_timestamp() };
        now = std::chrono::system_clock::time_point(dur);
    }

    TmxData pvt;
    pvt["track"] = get_value<heading::value_type>(bytes.substr(0, heading::numBytes)) / 1000000.0;
    bytes = bytes.substr(heading::numBytes);
    pvt["speed"] = get_value<speed::value_type>(bytes.substr(0, speed::numBytes)) / 1000.0;
    bytes = bytes.substr(speed::numBytes);
    pvt["lat"] = get_value<lat::value_type>(bytes.substr(0, lat::numBytes)) / 1000000.0 - 180.0;
    bytes = bytes.substr(lat::numBytes);
    pvt["lon"] = get_value<lon::value_type>(bytes.substr(0, lon::numBytes)) / 1000000.0 - 180.0;
    bytes = bytes.substr(lon::numBytes);
    pvt["altHAE"] = get_value<alt::value_type>(bytes.substr(0, alt::numBytes)) / 1000.0 - 500.0;
    pvt["time"] = std::regex_replace(utils::Clock::ToUtcPreciseTimeString(now), std::regex("\\s"), "T");
    pvt["mode"] = enums::enum_integer(message::v2x::FixTypes::ThreeD);
    pvt["status"] = enums::enum_integer(message::v2x::SignalQualityTypes::SimulationMode);
    pvt["device"] = std::string("/dev/v2x-sim");
    pvt["class"] = std::string("TPV");

    if (this->get_config("enable-sim-tpv"))
        this->broadcast(pvt.get_container(), "gpsd/TPV", __FUNCTION__);

    if (this->get_config("enable-sim-bsm")) {
        MessageFrame frame;
        auto &bsm = frame.value.choice.BasicSafetyMessage;
        frame.value.present = MessageFrame__value_PR_BasicSafetyMessage;

        auto nm = type_short_name(bsm).data();
        frame.messageId = v2x::MessageReceiver::get_message_id(nm);

        memset(&bsm, 0, sizeof(bsm));

        bsm.coreData.msgCnt = 0;

        std::uint8_t id[4];
        memcpy(id, &vehId, 4);
        bsm.coreData.id.size = 4;
        bsm.coreData.id.buf = id;

        bsm.coreData.secMark = (std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) -
                                std::chrono::duration_cast<std::chrono::minutes>(now.time_since_epoch())).count();

        // Latitude and Longitude are expressed in 1/10th integer microdegrees, as a 31 bit value.
        // The value 900000001 indicates that latitude is not available.
        // The value 1800000001 indicates that longitude is not available.
        bsm.coreData.lat = (Latitude_t) (10000000.0 * (double)pvt["lat"]);
        bsm.coreData.Long = (Longitude_t) (10000000.0 * (double)pvt["lon"]);

        // Elevation is in units of 10 cm steps.
        // max and min values are limited as set below.
        bsm.coreData.elev = 10.0 * (double)pvt["altHAE"];
        if (bsm.coreData.elev > 61439)
            bsm.coreData.elev = 61439;
        else if (bsm.coreData.elev < -4095)
            bsm.coreData.elev = -4095;

        // Convert from mps to .02 meters/sec.
        bsm.coreData.speed = 50.0 * (double)pvt["speed"];

        // Heading units are 0.0125 degrees.
        bsm.coreData.heading = 80.0 * (double)pvt["track"];
        if (bsm.coreData.heading > 28799)
            bsm.coreData.heading = 28799;
        else if (bsm.coreData.heading < 0)
            bsm.coreData.heading = 0;

        // Steering Wheel Angle units are 1.5 degrees (-126 to 127).
        bsm.coreData.angle = 127;
        bsm.coreData.accuracy.semiMajor = 0;
        bsm.coreData.accuracy.semiMinor = 0;
        bsm.coreData.accuracy.orientation = 0;

        bsm.coreData.transmission = TransmissionState_unavailable;

        bsm.coreData.accelSet.Long = 0;
        bsm.coreData.accelSet.lat = 0;
        bsm.coreData.accelSet.vert = 0;
        bsm.coreData.accelSet.yaw = 0;

        std::uint8_t wheelBrakes = 0;
        bsm.coreData.brakes.wheelBrakes.buf = &wheelBrakes;
        bsm.coreData.brakes.wheelBrakes.size = 1;
        bsm.coreData.brakes.wheelBrakes.bits_unused = 3;

        bsm.coreData.brakes.traction = TractionControlStatus_unavailable;
        bsm.coreData.brakes.abs = AntiLockBrakeStatus_unavailable;
        bsm.coreData.brakes.scs = StabilityControlStatus_unavailable;
        bsm.coreData.brakes.brakeBoost = BrakeBoostApplied_unavailable;
        bsm.coreData.brakes.auxBrakes = AuxiliaryBrakeStatus_unavailable;

        // Encode the bytes
        char buffer[4096];
        auto ret = uper_encode_to_buffer(&asn_DEF_MessageFrame, nullptr, &frame, buffer, 4096);
        if (ret.encoded > 0) {
            codec.get_message().set_timepoint(now);
            codec.get_message().set_source(std::string(pvt["device"].to_string().c_str()));
            codec.get_message().set_encoding("asn.1-uper");
            codec.get_message().set_payload(byte_string_encode(to_byte_sequence(buffer, ret.encoded / TMX_BITS_PER_BYTE)));
            this->invoke_handlers(types::String8(), codec.get_message(), "J2735/UNKNOWN");
        }
    }
}

template <>
void TmxPlugin::on_message_received<types::String8 const, v2x::MessageReceiver::simSRM>(types::String8 const &,
                                                                                        TmxMessage const &msg) {
    TLOG(DEBUG3) << "Enter " << TMX_PRETTY_FUNCTION << " with " << msg.to_string();

    auto bytes = byte_string_decode(msg.get_payload_string());

    // extract data
    // vehicleId(4), heading*M(4), speed*K(4), (latitude+180)*M(4), (longitude+180)*M(4), role (4)
    typedef UInt<32> vehicleId;
    typedef UInt<32> heading;
    typedef UInt<32> speed;
    typedef UInt<32> lat;
    typedef UInt<32> lon;
    typedef UInt<32> role;

    typedef bit_count<vehicleId, heading, speed, lat, lon, role> count;

    if (bytes.length() != count::value / TMX_BITS_PER_BYTE)
        return;

    auto vehId = get_value<vehicleId::value_type>(bytes.substr(0, vehicleId::numBytes));
    bytes = bytes.substr(vehicleId::numBytes);

/*
    auto now = std::chrono::system_clock::now();

    auto track = get_value<heading::value_type>(bytes.substr(0, heading::numBytes));
    bytes = bytes.substr(heading::numBytes);
    auto spd = get_value<speed::value_type>(bytes.substr(0, speed::numBytes));
    bytes = bytes.substr(speed::numBytes);
    auto latitude = get_value<lat::value_type>(bytes.substr(0, lat::numBytes)) / 1000000.0 - 180.0;
    bytes = bytes.substr(lat::numBytes);
    auto longitude = get_value<lon::value_type>(bytes.substr(0, lon::numBytes)) / 1000000.0 - 180.0;
    bytes = bytes.substr(lon::numBytes);
    auto r = get_value<role::value_type>(bytes.substr(0, role::numBytes)) / 1000.0 - 500.0;

    TLOG(DEBUG1) << pvt.to_string();

*/
    MessageFrame frame;
    auto &srm = frame.value.choice.SignalRequestMessage;
    frame.value.present = MessageFrame__value_PR_SignalRequestMessage;

    auto nm = type_short_name(srm).data();
    frame.messageId = v2x::MessageReceiver::get_message_id(nm);

    memset(&srm, 0, sizeof(srm));

//    srm.requestor.id.present = VehicleID_PR_entityID;
//    srm.requestor.id.choice.entityID.size = s;
//    srm.requestor.id.choice.entityID.buf = (uint8_t *)calloc(s, sizeof(uint8_t));
//    if (srm->requestor.id.choice.entityID.buf)
//        memcpy(srm->requestor.id.choice.entityID.buf, &vehicleId, s);
//
//        srm->requestor.type =
//                (struct RequestorType *)calloc(1, sizeof(struct RequestorType));
//        if (srm->requestor.type)
//        {
//            srm->requestor.type->role = (BasicVehicleRole_t)role;
//            srm->requestor.position =
//                    (struct RequestorPositionVector *)calloc(1, sizeof(struct RequestorPositionVector));
//            if (srm->requestor.position)
//            {
//                srm->requestor.position->position.lat = (Latitude_t)(10.0 * latitude - 1800000000);
//                srm->requestor.position->position.Long = (Longitude_t)(10.0 * longitude - 1800000000);
//                srm->requestor.position->heading =
//                        (DSRC_Angle_t *)calloc(1, sizeof(DSRC_Angle_t));
//                if (srm->requestor.position->heading)
//                    *(srm->requestor.position->heading) = (DSRC_Angle_t)(heading / 12500.0);
//                srm->requestor.position->speed =
//                        (TransmissionAndSpeed *)calloc(1, sizeof(TransmissionAndSpeed));
//                if (srm->requestor.position->speed)
//                {
//                    srm->requestor.position->speed->transmisson = TransmissionState_unavailable;
//                    srm->requestor.position->speed->speed = (Velocity_t)(speed / 20.0);
//                }
//            }
//        }
//    }


}

template <>
void TmxPlugin::on_message_received<types::String8 const, v2x::MessageReceiver::simVBM>(types::String8 const &,
                                                                                        TmxMessage const &msg) {
    TLOG(DEBUG3) << "Enter " << TMX_PRETTY_FUNCTION << " with " << msg.to_string();

    auto bytes = byte_string_decode(msg.get_payload_string());

    //extract data
    // vehicleId(4), speed*K(4), gearPosition(1), turnSignalPosition(1), flags1(1), acceleration*K(4)
    typedef UInt<32> vehicleId;
    typedef UInt<32> speed;
    typedef UInt<8> gearPosition;
    typedef UInt<8> turnSignalPosition;
    typedef UInt<8> flags1;
    typedef UInt<32> acceleration;

    typedef bit_count<vehicleId, speed, gearPosition, turnSignalPosition, flags1, acceleration> count;

    if (bytes.length() != count::value / TMX_BITS_PER_BYTE)
        return;

    auto vehId = get_value<vehicleId::value_type>(bytes.substr(0, vehicleId::numBytes));
    bytes = bytes.substr(vehicleId::numBytes);

}

namespace v2x {
namespace MessageReceiver {

std::atomic<uint64_t> MessageReceiverPlugin::_startTime{0};
std::atomic<uint64_t> MessageReceiverPlugin::_totalBytes{0};
std::map<std::string, std::atomic<uint32_t>> MessageReceiverPlugin::_totalCount;
std::mutex MessageReceiverPlugin::_mapMutex; // Define the mutex

MessageReceiverPlugin::MessageReceiverPlugin() {
    // Register handlers
    this->register_handler<incoming>("UNKNOWN", dynamic_cast<TmxPlugin *>(this),
                                     &TmxPlugin::on_message_received<TmxData const, incoming>);
    this->register_handler<TmxPluginDataUpdate>(this->get_topic("config/status-Hz"), this,
                                                &MessageReceiverPlugin::handle_config_update);

	_errThrottle.set_Frequency(std::chrono::seconds(ERROR_WAIT_MS));
	_statusThrottle.set_Frequency(std::chrono::seconds(STATUS_WAIT_MS));
}

MessageReceiverPlugin::~MessageReceiverPlugin() = default;

common::TmxTypeDescriptor MessageReceiverPlugin::get_descriptor() const noexcept {
    auto _descr = TmxPlugin::get_descriptor();
    return { _descr.get_instance(), typeid(*this), type_fqname(*this).data() };
}

common::types::Array<common::types::Any> MessageReceiverPlugin::get_config_description() const noexcept {
    message::TmxData _data;
    auto decoder = message::codec::TmxDecoder::get_decoder("json");
    if (decoder)
        decoder->decode(_data.get_container(), to_byte_sequence(MESSAGE_RECEIVER_CONFIGURATION_JSON));
    return _data.to_array();
}

void MessageReceiverPlugin::init() {
    TmxPlugin::init();

    // Internal handlers
    this->register_handler<j2735>("J2735/UNKNOWN", dynamic_cast<TmxPlugin *>(this),
                                  &TmxPlugin::on_message_received<types::String8 const, j2735>);
    this->register_handler<simBSM>("Simulated/BSM", dynamic_cast<TmxPlugin *>(this),
                                   &TmxPlugin::on_message_received<types::String8 const, simBSM>);
    this->register_handler<simSRM>("Simulated/SRM", dynamic_cast<TmxPlugin *>(this),
                                   &TmxPlugin::on_message_received<types::String8 const, simSRM>);
    this->register_handler<simVBM>("Simulated/VBM", dynamic_cast<TmxPlugin *>(this),
                                   &TmxPlugin::on_message_received<types::String8 const, simVBM>);
}

void MessageReceiverPlugin::handle_config_update(TmxPluginDataUpdate const &data, tmx::message::TmxMessage const &msg) {
    TLOG(DEBUG) << "handle_config_updated invoked with " << data.get_container();

    if (strcmp("status-Hz", data.get_key().c_str()) == 0) {
        typedef std::chrono::duration<double, std::chrono::seconds::period> persec;
        std::lock_guard<std::mutex> _lock(this->_dataLock);
        _statusThrottle.set_Frequency(persec(1.0 / data.get_value().to_float()));

        TLOG(DEBUG) << "status-Hz set to " <<
                    std::chrono::duration_cast<std::chrono::seconds>(_statusThrottle.get_Frequency()).count() << " sec";
    }
}

common::TmxError MessageReceiverPlugin::main() noexcept
{
    TLOG(DEBUG) << "Starting " << this->get_descriptor().get_type_name();
    this->set_status("State", "Running");
    _startTime = Clock::GetMillisecondsSinceEpoch();

    while(this->is_running()) {
        if (_statusThrottle.Monitor(1)) {
            uint64_t b = _totalBytes;
            auto msCount = Clock::GetMillisecondsSinceEpoch() - _startTime;

            this->set_status("Total KBytes Received", b / 1024.0);

            for (auto iter = _totalCount.begin(); iter != _totalCount.end(); iter++) {
                string param("Avg ");
                param += iter->first;
                param += " Message Interval (ms)";

                uint32_t c = _totalCount[iter->first];
                this->set_status(param.c_str(), c == 0 ? 0 : 1.0 * msCount / c);

                param = "Total ";
                param += iter->first;
                param += " Messages Received";
                this->set_status(param.c_str(), c);
            }
        }
    }

    this->set_status("State", "Terminated");

	return 0;
}

} /* End namespace MessageReceiver */
} /* End namespace v2x */
} /* End namespace plugin */
} /* End namespace tmx */

int main(int argc, char *argv[])
{
    tmx::plugin::v2x::MessageReceiver::MessageReceiverPlugin _plugin;
	return run(_plugin, argc, argv);
}
