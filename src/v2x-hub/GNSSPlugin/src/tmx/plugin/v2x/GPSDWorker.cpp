/*
 * @file GPSDWorker.cpp
 *
 *  Created on: Jul 21, 2020
 *      @author: Gregory M. Baumgardner
 *  Edited on: May 16, 2024
 *      @author: Noah Limes
 */

#include <tmx/plugin/v2x/GNSSPlugin.hpp>
#include <tmx/plugin/v2x/GPSDWorker.hpp>
#include <tmx/plugin/v2x/GPSDBrokerClient.hpp>

#include <tmx/plugin/utils/Clock.hpp>
#include <tmx/plugin/utils/FrequencyThrottle.hpp>
#include <tmx/plugin/utils/System.hpp>
#include <tmx/plugin/utils/Uuid.hpp>
#include <tmx/message/v2x/LocationMessage.hpp>
#include <tmx/message/v2x/LocationMessageEnumTypes.hpp>
#include <tmx/message/v2x/rtcm/RtcmMessage.hpp>

#include <array>
#include <atomic>
#include <bitset>
#include <cctype>
#include <cerrno>
#include <cstdlib>
#include <gps.h>
#include <mutex>
#include <regex>
#include <sstream>
#include <sys/select.h>
#include <sys/socket.h>
#include <vector>

using namespace tmx::broker;
using namespace tmx::broker::gpsd;
using namespace tmx::common;
using namespace tmx::common::types;
using namespace tmx::message;
using namespace tmx::message::location;
//using namespace tmx::messages::rtcm;
using namespace tmx::plugin::utils;

namespace tmx {
namespace plugin {
namespace v2x {
namespace GNSS {

static FrequencyThrottle <size_t, std::chrono::milliseconds> _sampleThrottle{std::chrono::milliseconds(20)};
static FrequencyThrottle<int, std::chrono::seconds> _statusThrottle{std::chrono::seconds(3)};

static std::atomic<bool> _detectRtcm2{false};
static std::atomic<bool> _detectRtcm3{false};
static std::atomic<bool> _detectUblox{false};
static std::atomic<bool> _detect3DFix{false};
static std::atomic<bool> _detectDGPS{false};

static std::string baseCmd = "PATH=$PATH:/usr/bin:/usr/local/bin ubxtool ";

static auto &get_connections() noexcept {
    static std::array<TmxBrokerContext, 10> _singleton;
    return _singleton;
};

static auto get_plugin() noexcept {
    static std::shared_ptr<GNSSPlugin> _plugin;
    if (!_plugin)
        _plugin = std::static_pointer_cast<GNSSPlugin>(TmxPlugin::get_plugin("GNSS"));
    return _plugin;
};

static auto get_broker(TmxBrokerContext const &ctx) noexcept {
    static std::shared_ptr<GPSDBrokerClient> _broker;
    if (!_broker)
        _broker = std::static_pointer_cast<GPSDBrokerClient>(TmxBrokerClient::get_broker(ctx));
    return _broker;
}

template<typename _Msg>
typename std::enable_if<std::is_base_of<GPSDMessage, _Msg>::value, common::const_string>::type
get_msg_topic(_Msg const &) {
    static std::string _nmspace{(TmxTypeRegistry(_Msg::main_topic) / _Msg::topic).get_namespace()};
    return _nmspace;
}


// Need a temporary class to correctly set missing data for RTCM messages
// coming from the GPSD

FixTypes get_fix(int fix) {
    switch (fix) {
        case MODE_NO_FIX:
            return FixTypes::NoFix;
        case MODE_2D:
            return FixTypes::TwoD;
        case MODE_3D:
            return FixTypes::ThreeD;
        default:
            return FixTypes::Unknown;
    }
}

SignalQualityTypes get_quality(int quality) {
    switch (quality) {
        case STATUS_GPS:
            return SignalQualityTypes::GPS;
        case STATUS_RTK_FLT:
            return SignalQualityTypes::FloatRTK;
        case STATUS_RTK_FIX:
            return SignalQualityTypes::RealTimeKinematic;
        case STATUS_DGPS:
            return SignalQualityTypes::DGPS;
        case STATUS_DR:
        case STATUS_GNSSDR:
            return SignalQualityTypes::DeadReckoning;
        case STATUS_TIME:
            return SignalQualityTypes::ManualInputMode;
        case STATUS_SIM:
            return SignalQualityTypes::SimulationMode;
        case STATUS_PPS_FIX:
            return SignalQualityTypes::PPS;
        default:
            return SignalQualityTypes::Invalid;
    }
}

common::TmxError GPSDWorker::execute(TmxBrokerContext &ctx, Any const &data) const noexcept {
    auto _sMsg = tmx::common::any_cast<tmx::messages::GPSDSampleMessage>(&data);
    if (_sMsg) return this->handle_sample_message(ctx, *_sMsg);

    auto _vMsg = tmx::common::any_cast<tmx::messages::GPSDVersionMessage>(&data);
    if (_vMsg) return this->handle_version_message(ctx, *_vMsg);

    auto _dMsg = tmx::common::any_cast<tmx::messages::GPSDDeviceMessage>(&data);
    if (_dMsg) return this->handle_device_message(ctx, *_dMsg);

    auto _nMsg = tmx::common::any_cast<tmx::messages::GPSDNAVPVTMessage>(&data);
    if (_nMsg) return this->handle_navpvt_message(ctx, *_nMsg);

    auto _rMsg = tmx::common::any_cast<tmx::messages::GPSDDecodeRawMessage>(&data);
    if (_rMsg) return this->handle_decoderaw_message(ctx, *_rMsg);

    return {1, "Unknown GPSD message type"};
}

std::string ToString(FixTypes fix)
{
    switch (fix)
    {
        case FixTypes::TwoD: return "2D";
        case FixTypes::ThreeD: return "3D";
        default: return Enum<FixTypes>(fix).get_enum_name();
    }
}

std::string ToString(SignalQualityTypes quality)
{
    switch (quality)
    {
        case SignalQualityTypes::RealTimeKinematic: return "RTK";
        default: return Enum<SignalQualityTypes>(quality).get_enum_name();
    }
}

void BroadcastLocation(LocationMessage &loc) {
    // This function pieces together disjoint parts of the location message
    static LocationMessage _tmp;
    if (loc.get_Id().empty()) {
        _tmp = loc;
        return;
    } else if (!_detectDGPS) {
        // Reset the quality measure until new data is needed
        _tmp.set_SignalQuality(loc.get_SignalQuality());
    }

    if (loc.get_SignalQuality() < _tmp.get_SignalQuality())
        loc.set_SignalQuality(_tmp.get_SignalQuality());

    typename Properties<Any>::value_type _props = {
            { "Id", loc.get_Id() },
            { "SignalQuality", (int)loc.get_SignalQuality() },
            { "FixQuality", (int)loc.get_FixQuality() },
            { "Latitude", loc.get_Latitude() },
            { "Longitude", loc.get_Longitude() },
            { "Altitude", loc.get_Altitude() },
            { "Time", loc.get_Time() },
            { "SentenceIdentifier", loc.get_SentenceIdentifier() },
            { "HorizontalDOP", loc.get_HorizontalDOP() },
            { "NumSatellites", loc.get_NumSatellites() },
            { "Speed", loc.get_Speed() },
            { "Heading", loc.get_Heading() }
    };
    auto plugin = get_plugin();
    if (plugin)
        plugin->broadcast(_props, "V2X/Location", plugin->get_descriptor().get_type_name(), "json");
}

/**
 * There are three separate worker threads for GPSD.
 * Note that each require separate types of streams from the GPSD server.
 * When adding them all to the same thread, the raw streams consume most
 * of the CPU, thus starving out the Location message generator. Therefore,
 * it was a better design to separate the connections with different sockets
 * running in different threads.
 *
 * 1) The Location message generator, which is required
 * 2) The NMEA message generator, when selected
 * 3) The RAW bytes message reader, for use in scanning proprietary
 *    u-Blox messages and for other stuff like RTCM
 */
common::TmxError sample_json(TmxBrokerContext &ctx) noexcept {
    static std::uint64_t lastTime = 0;
    static std::string lastDev;

    static double _lastLat = 0.0;
    static double _lastLon = 0.0;
    static double _lastAlt = 0.0;
    static std::uint64_t _lastHeading = 0;

    struct gps_data_t *gps = tmx::common::any_cast<struct gps_data_t>(&(ctx.at(ctx.get_scheme())));
    if (!gps)
        return {2, "Context not properly initialized for GPS use."};

    // TODO: Listen for updates
    double _latchSpeed = 0.1;
    auto plugin = get_plugin();

    int ret = gps_read(gps, NULL, 0);
    if (ret < 0) {
        std::string err{"Error reading from GPS stream: "};
        err.append(gps_errstr(ret));
        return {1, err};
    }

    if (gps->online.tv_sec || gps->online.tv_nsec) {
        std::uint64_t ms = Clock::GetMillisecondsSinceEpoch(gps->fix.time);

        bool timeUpd = false;
        {
            std::lock_guard<std::mutex> lock(ctx.get_thread_lock());
            timeUpd = ms > lastTime;
            lastTime = ms;
        }

        if (gps->set & MODE_SET && timeUpd) {
            // Send the location message
            LocationMessage loc{Uuid::NewGuid(),
                                get_quality(gps->fix.status), "", ms,
                                gps->fix.latitude, gps->fix.longitude,
                                get_fix(gps->fix.mode), gps->satellites_used,
                                gps->dop.hdop, gps->fix.speed,
                                gps->fix.track};

            if (gps->set & ALTITUDE_SET)
                loc.set_Altitude(gps->fix.altHAE);

            _detect3DFix = (loc.get_FixQuality() >= FixTypes::ThreeD);

            if (loc.get_FixQuality() > FixTypes::NoFix) {
                _detectDGPS = (loc.get_SignalQuality() == SignalQualityTypes::DGPS);

                // Latch the heading and position if the vehicle slows below minimum speed
                // But, ignore latching for stationary
                if (loc.get_SignalQuality() == SignalQualityTypes::ManualInputMode) {
                    // Do nothing
                    ;
                } else if (_latchSpeed > loc.get_Speed_mph()) {
                    loc.set_Latitude(_lastLat);
                    loc.set_Longitude(_lastLon);
                    loc.set_Altitude(_lastAlt);
                    loc.set_Heading((double) _lastHeading);
                    loc.set_Speed_mps(0.0);
                } else {
                    _lastLat = loc.get_Latitude();
                    _lastLon = loc.get_Longitude();
                    _lastAlt = loc.get_Altitude();
                    _lastHeading = loc.get_Heading();
                }

                // Broadcast immediately
                BroadcastLocation(loc);
            } else {
                _lastLat = 0.0;
                _lastLon = 0.0;
                _lastAlt = 0.0;
                _lastHeading = 0;
            }

            if (plugin && _statusThrottle.Monitor(0x00)) {
                plugin->set_status("Last_GPS_Time", Clock::ToLocalPreciseTimeString(
                        Clock::GetTimepointSinceEpoch(ms)));
                plugin->set_status("Current_Time", Clock::ToLocalPreciseTimeString(
                        Clock::GetTimepointSinceEpoch(Clock::GetMillisecondsSinceEpoch())));
                //SetStatus("GPS", (ctrl.get_gps_device().empty() ? "N/A" :
                //	(ctrl.get_gps_type().empty() ? "Unknown type" : ctrl.get_gps_type()) + " on " + ctrl.get_gps_device()));
                plugin->set_status("Number_of_Satellites", loc.get_NumSatellites());
                plugin->set_status("Fix_Quality", ToString(loc.get_FixQuality()));
                plugin->set_status("Signal_Quality", ToString(loc.get_SignalQuality()));
                if (loc.get_FixQuality() > FixTypes::NoFix) {
                    plugin->set_status("Altitude", loc.get_Altitude());
                    plugin->set_status("Latitude", loc.get_Latitude());
                    plugin->set_status("Longitude", loc.get_Longitude());
                    plugin->set_status("Speed_MPH", loc.get_Speed_mph());
                    plugin->set_status("Heading", loc.get_Heading());
                    plugin->set_status("HDOP", loc.get_HorizontalDOP());
                } else {
                    plugin->set_status("Altitude", "N/A");
                    plugin->set_status("Latitude", "N/A");
                    plugin->set_status("Longitude", "N/A");
                    plugin->set_status("Speed_MPH", "N/A");
                    plugin->set_status("Heading", "N/A");
                    plugin->set_status("HDOP", "N/A");
                }
            }
        }

        auto broker = get_broker(ctx);

        // Decode the GPSD version data, if available
        if (gps->set & VERSION_SET) {
            messages::GPSDVersionMessage ver(gps->version.release);
            if (broker)
                broker->publish(ctx, get_msg_topic(ver), ver);
        }

        if (gps->set & RTCM2_SET) {
            _detectRtcm2 = true;
        }

        if (gps->set & RTCM3_SET) {
            _detectRtcm3 = true;
        }

        // Decode the GPSD device data
        for (size_t i = 0; lastDev != gps->dev.path && i < gps->devices.ndevices; i++) {
            if (strcmp(gps->dev.path, gps->devices.list[i].path) == 0) {
                messages::GPSDDeviceMessage dev;
                dev.set_path(gps->devices.list[i].path);
                dev.set_driver(gps->devices.list[i].driver);
                dev.set_flags(gps->devices.list[i].flags);
                dev.set_activated(Clock::GetMillisecondsSinceEpoch(gps->devices.list[i].activated));
                dev.set_subtype(gps->devices.list[i].subtype);
                dev.set_subtype1(gps->devices.list[i].subtype1);

                if (broker)
                    broker->publish(ctx, get_msg_topic(dev), dev);
            }
        }
    }

    return { };
}

common::TmxError sample_nmea(TmxBrokerContext &ctx) {
    static std::regex ggaExpr {"\\$G[LNP]GGA,.*"};
    static std::regex gsaExpr {"\\$G[LNP]GSA,.*"};
    static std::regex rmcExpr {"\\$G[LNP]RMC,.*"};
    static std::regex gsvExpr {"\\$G[LNP]GSV,.*"};
    static std::regex vtgExpr {"\\$G[LNP]VTG,.*"};
    static std::regex gllExpr {"\\$G[LNP]GLL,.*"};

    // Send RAW NMEA strings
    struct gps_data_t *gps = tmx::common::any_cast<struct gps_data_t>(&(ctx.at(ctx.get_scheme())));
    if (!gps)
        return {2, "Context not properly initialized for GPS use."};

    int ret = gps_read(gps, NULL, 0);
    if (ret < 0) {
        std::string err{"Error reading from GPS stream: "};
        err.append(gps_errstr(ret));
        return {1, err};
    }

    auto plugin = get_plugin();

    if (gps->online.tv_sec || gps->online.tv_nsec) {
        std::istringstream is(std::string(gps_data(gps), ret));
        for (std::string line; std::getline(is, line);) {
            if (line.empty())
                continue;

            if (line[line.length() - 1] == '\r')
                line.erase(line.length() - 1);
            std::cout << "Scanning '" << line << "'" << std::endl;

            std::string topic;
            TmxTypeRegistry _base { "V2X/Location" };

            if(std::regex_match(line, ggaExpr))
                topic = (_base / "GGA").get_namespace().data();
            if(std::regex_match(line, gsaExpr))
                topic = (_base / "GSA").get_namespace().data();
            if(std::regex_match(line, rmcExpr))
                topic = (_base / "RMC").get_namespace().data();
            if(std::regex_match(line, gsvExpr))
                topic = (_base / "GSV").get_namespace().data();
            if(std::regex_match(line, vtgExpr))
                topic = (_base / "VTG").get_namespace().data();
            if(std::regex_match(line, gllExpr))
                topic = (_base / "GLL").get_namespace().data();

            if (!topic.empty() && plugin)
                plugin->broadcast(line, topic, plugin->get_descriptor().get_type_name(), "json");
        }
    }

    return { };
}

common::TmxError sample_raw(TmxBrokerContext &ctx) {
    static std::string rawString;

    struct gps_data_t *gps = tmx::common::any_cast<struct gps_data_t>(&(ctx.at(ctx.get_scheme())));
    if (!gps)
        return {2, "Context not properly initialized for GPS use."};

    std::string bytes(4096, '\0');

    auto broker = get_broker(ctx);
    if (gps->gps_fd > 0) {
        int r = read(gps->gps_fd, bytes.data(), bytes.size());
        if (r > 0) {
            if (r < bytes.size()) bytes.resize(r);

            rawString.append(bytes.begin(), bytes.end());

            // Go up to the last newline
            size_t lastNL = rawString.find_last_of('\n');
            size_t len = rawString.length() - (lastNL < rawString.length() ? lastNL : 0);
            bytes.resize(len);
            memcpy(bytes.data(), rawString.c_str(), len);
            if (len == rawString.length())
                rawString.clear();
            else
                rawString.erase(rawString.begin(), rawString.begin() + len);

            messages::GPSDDecodeRawMessage rawMsg;
            rawMsg.set_bytes(byte_string_encode(to_byte_sequence(bytes.data())));

            std::cout << rawMsg.get_bytes() << std::endl;
            if (broker)
                broker->publish(ctx, get_msg_topic(rawMsg), rawMsg);
        }
    }

    return { };
}


common::TmxError GPSDWorker::handle_sample_message(TmxBrokerContext &ctx, messages::GPSDSampleMessage const &sample) const noexcept {
    std::cout << "Sampling " << sample.get_connectionId() << std::endl;

    switch (sample.get_connectionId()) {
        case WATCH_JSON:
            return sample_json(ctx);
        case WATCH_NMEA:
            return sample_nmea(ctx);
        case WATCH_RAW:
            return sample_raw(ctx);
        default:
            break;
    }
    return { };
}

common::TmxError GPSDWorker::handle_version_message(TmxBrokerContext &ctx, messages::GPSDVersionMessage const &ver) const  noexcept {
    typedef std::chrono::system_clock::duration duration;
    std::cout << "Connected to GPSD version " << ver.get_version() << std::endl;

    if (get_plugin())
        get_plugin()->set_status("GPSD_Version", ver.get_version());

    return { };
}

common::TmxError GPSDWorker::handle_device_message(TmxBrokerContext &ctx, messages::GPSDDeviceMessage const &msg) const noexcept {
    std::cout << "Connected to GPSD device " << msg.get_path() << " on " << ctx.get_host() << std::endl;

    return { };
}


common::TmxError GPSDWorker::handle_navpvt_message(TmxBrokerContext &, tmx::messages::GPSDNAVPVTMessage const &msg) const noexcept {
    return { };
}
/*
void decodeNAVPVT(std::string byteStr, GNSSPlugin *plugin) {
    static std::string ubxNAVPVTPattern = "b56201075c00";
    static auto ubxNAVPVTBytes = byte_string_decode(ubxNAVPVTPattern);
    static size_t ubxNAVPVTLength = (size_t) ubxNAVPVTBytes[ubxNAVPVTBytes.size() - 2];

    std::regex exp(ubxNAVPVTPattern + "[a-zA-Z0-9]{" + std::to_string(ubxNAVPVTLength) + "}");
    std::regex_token_iterator<std::string::iterator> rend;
    std::regex_token_iterator<std::string::iterator> tokenizer { byteStr.begin(), byteStr.end(), exp };

    for ( ; tokenizer != rend; tokenizer++) {
        // This uBlox message is used to check for RTK float or fixed,
        // so pass to the Location decoding thread
        message::codec::TmxCodec _codec;
        _codec.encode(tokenizer->str(), "json");
        _codec.get_message()
        messages::GPSDNAVPVTMessage navPvt;
        routeable_message rMsg;
        rMsg.initialize(navPvt, "Internal");
        msgBytes = byte_stream_decode(tokenizer->str());
        rMsg.set_payload_bytes(msgBytes);
        rMsg.reinit();

        FILE_LOG(logDEBUG1) << this_thread::get_id() << ": Sending: " << rMsg;
        plugin->IncomingMessage(rMsg, GPSD_GROUP, WATCH_JSON);
    }
}
*/
TmxError GPSDWorker::handle_decoderaw_message(TmxBrokerContext &ctx, GPSDDecodeRawMessage const &msg) const noexcept {
/*   if (_detectRtcm3)
        decodeRtcm<rtcm::RTCM_VERSION::SC10403_3>(tmxMsg.get_payload(), this);
    if (_detectRtcm2)
        decodeRtcm<rtcm::RTCM_VERSION::SC10402_3>(tmxMsg.get_payload(), this);
    if (_detectDGPS && _detectUblox)
        decodeNAVPVT(rMsg.get_payload_str(), this);
        */

    return { };
}
/*
void GNSSPlugin::configure_device() {
	static vector<string> cfgOptions;

	string cmd;
	string out;

	// Currently only valid for u-Blox devices
	if (_detectUblox) {
		if (!cfgOptions.size()) {
			cmd = baseCmd + "-h -v5 | awk '$1 ~ /CFG-.*-/ {print $1}'";
			out = System::ExecCommand(cmd);

			istringstream is(out);
			for (string line; std::getline(is, line); ) {
				if (line.empty())
					continue;

				if (line[line.length() - 1] == '\r')
					line.erase(line.length() - 1);

				cfgOptions.push_back(line);
			}
		}

		Array<GPSDUBXConfigMessage> cfgArray;
		for (string opt: cfgOptions) {
			auto val = as<std::string>(this->get_config(opt, &_dataLock));
            if (val) {
                GPSDUBXConfigMessage cfg;
                cfgArray.emplace_back(cfg);
                cfgArray.back().set_parameter(opt);
                cfgArray.back().set_value(*val);
            }
		}

        message::codec::TmxCodec _codec;
        _codec.get_message().set_id(type_fqname(cfgArray).data());
        _codec.get_message().set_source(__FUNCTION__);
        _codec.encode(cfgArray, "json");

		this->on_message_received(_codec.get_message());
	}
}



*/
/*

bool isRtcmNew(RTCM2Message *msg) {
	static uint16_t lastCRC1 = 0;
	static uint16_t lastCRC2 = 0;

	if (!msg) return false;

	if (lastCRC1 == msg->get_Parity1() &&
		lastCRC2 == msg->get_Parity2()) {
		return false;
	}

	lastCRC1 = msg->get_Parity1();
	lastCRC2 = msg->get_Parity2();
	return true;
}

bool isRtcmNew(RTCM3Message *msg) {
	static uint32_t lastCRC;

	if (!msg) return false;

	if (lastCRC == msg->get_CRC())
		return false;

	lastCRC = msg->get_CRC();
	return true;
}

template <>
void doSample<0x10000u>(uint16_t connection, GNSSPlugin *_plugin) { }

size_t lastSampled(byte_t id) {
	static map<size_t, uint64_t> lastSampled;
	if (!lastSampled.count(id))
		lastSampled[id] = 0;

	return lastSampled[id];
}
*/
/**
 * Decode and process the incoming messages. This function deals mainly
 * with the internal GPSD messages, and passes the others on up the chain
 */
/*void GNSSPlugin::on_message_received(const Any &msg) {
	if (contains<messages::GPSDSampleMessage>(msg)) {
		messages::GPSDSampleMessage sample = rMsg.get_payload<messages::GPSDSampleMessage>();
		doSample<1>(sample.get_connectionId(), this);
	} else if (contains<messages::GPSDVersionMessage>(msg)) {
		messages::GPSDVersionMessage ver = rMsg.get_payload<messages::GPSDVersionMessage>();
		FILE_LOG(logINFO) << "Connected to GPSD version " << ver.get_version();
	} else if (IsMessageOfType<messages::GPSDDeviceMessage>(ivpMsg)) {
		messages::GPSDDeviceMessage dev = rMsg.get_payload<messages::GPSDDeviceMessage>();

		string driver = dev.get_driver();

		// Need to configure the device if the driver was newly discovered as a u-Blox
		if (!_detectUblox.exchange((driver == "u-blox")))
			ConfigureDevice();

		if (_statusThrottle.Monitor(0xDEF1CE)) {
			string type = dev.get_subtype1();
			if (!type.empty()) {
				auto loc = type.find("MOD=");
				if (loc != string::npos) {
					type = type.substr(loc+4);
					loc = type.find(",");
					if (loc != string::npos) {
						type = type.substr(0, loc);
					}
				}

				type = " " + type;
			}
			SetStatus("GPS Device Type", driver + (type.empty() ? "" : type));
			SetStatus("GPS Device Path", dev.get_path());
			SetStatus("Connected Since", Clock::ToLocalPreciseTimeString(Clock::GetTimepointSinceEpoch(dev.get_activated())));
		}
	} else if (IsMessageOfType<messages::GPSDUBXConfigMessage>(ivpMsg)) {
		string cmd = baseCmd;
		string host = GPSDConnection::connection(WATCH_JSON).get_host();

		messages::GPSDUBXConfigMessage cfgArray = rMsg.get_payload<messages::GPSDUBXConfigMessage>();
		for (messages::GPSDUBXConfigMessage cfg: cfgArray.get_array<messages::GPSDUBXConfigMessage>(cfgArray.ArrayElement)) {
			cmd.append("-z ");
			cmd.append(cfg.get_parameter());
			cmd.append(",");
			cmd.append(cfg.get_value());
			cmd.append(" ");
		}

		if (cmd != baseCmd) {
			cmd += host;
			PLOG(logDEBUG) << "Configuring u-Blox device with " << cmd;
			System::ExecCommand(cmd);

			PLOG(logDEBUG) << "Saving u-Blox device configuration with " << cmd;
			cmd = baseCmd + " -p SAVE " + host;
			string out = System::ExecCommand(cmd);
			PLOG(logDEBUG) << out;
		}
	} else if (IsMessageOfType<messages::GPSDNAVPVTMessage>(ivpMsg)) {
		// For a u-blox RTK device, the RTK float and fixed-integer modes are reported as DGPS.
		// Need to check the UBX-NAV-PVT message flags for the carrier signal state
		// See the Interface Description document at www.u-blox.com
		SignalQualityTypes quality = SignalQualityTypes::Invalid;

		// Extract the message and decode the flags
		byte_stream bytes = rMsg.get_payload_bytes();
		if (bytes.size() >= 28) {
			byte_t b = bytes[27];
			if ((b & 0x40) == 0x04)
				quality = SignalQualityTypes::FloatRTK;
			else if ((b & 0x80) == 0x08)
				quality = SignalQualityTypes::RealTimeKinematic;
			else if ((b & 0x02) == 0x02)
				quality = SignalQualityTypes::DGPS;
		}

		if (quality > SignalQualityTypes::Invalid) {
			LocationMessage tmp;
			tmp.set_SignalQuality(quality);
			BroadcastLocation(tmp, this);
		}
	} else if (IsMessageOfType<messages::GPSDDecodeRawMessage>(ivpMsg)) {
		byte_stream bytes = rMsg.get_payload_bytes();

		if (_detectRtcm3)
			decodeRtcm<rtcm::RTCM_VERSION::SC10403_3>(bytes, this);
		if (_detectRtcm2)
			decodeRtcm<rtcm::RTCM_VERSION::SC10402_3>(bytes, this);
		if (_detectDGPS && _detectUblox)
			decodeNAVPVT(rMsg.get_payload_str(), this);

	} else {
		// Pass it up the chain
		TmxMessageManager::OnMessageReceived(msg);
	}
}
*/

static TmxTypeRegistrar<GPSDWorker> _worker;

void GNSSPlugin::sample_gpsd() {
    std::this_thread::sleep_for(_sampleThrottle.get_Frequency());

    std::string host;
    {
        std::lock_guard<std::mutex> _lock(this->_dataLock);
        host = this->_gpsdHost;
    }

    if (host.empty())
        return;

    if (TmxBrokerContext("host").get_scheme().empty())
        host = "gps://" + host;

	int r;
	struct timeval tv;

	fd_set fds;
	FD_ZERO(&fds);

	tv.tv_sec = 0;
	tv.tv_usec = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::milliseconds(
            _sampleThrottle.get_Frequency())).count();

	std::bitset<16> mask(WATCH_JSON);
	if (_sendNMEA)
		mask |= WATCH_NMEA;
	if (_detectRtcm2 || _detectRtcm3)
		mask |= WATCH_RAW;
	else if (_detectUblox && _detectDGPS)
		mask |= WATCH_RAW;

	int nfds = 0;
	for (typename UInt32::value_type i: { WATCH_JSON, WATCH_NMEA, WATCH_RAW }) {
        auto &ctx = get_connections()[(i >> 4)];

        bool notNeeded = !(mask.to_ulong() & i);
        bool newHost = ctx.to_string() != host;

        // Clean up any open connections that are no longer needed or have changed hosts
        if (ctx && (notNeeded || newHost)) {
            auto broker = get_broker(ctx);
            if (broker && ctx.get_state() != TmxBrokerState::uninitialized) {
                if (i == WATCH_JSON)
                    this->set_status("GPSD_connection_state", "Disconnected");

                broker->disconnect(ctx);
                broker->destroy(ctx);
            }

            ctx.clear();
        }

        // Skip this connection if it is not needed
        if (notNeeded)
            continue;

        // Connect to the host
        if (newHost)
            ctx = TmxBrokerContext(host, { });

        auto broker = get_broker(ctx);
        if (!broker) {
            if (_statusThrottle.Monitor(0))
                std::cout << "ERROR: Unable to get broker for " << ctx.to_string() << std::endl;
            break;
        }

        if (!broker->is_connected(ctx)) {
            if (i == WATCH_JSON)
                this->set_status("GPSD_connection_state", "Connecting...");

            if (ctx.get_state() == TmxBrokerState::uninitialized)
                broker->initialize(ctx);
            broker->connect(ctx, i);

            if (i == WATCH_JSON)
                this->set_status("GPSD_connection_state", (broker->is_connected(ctx) ? "Connected" : "Disconnected"));

            // Subscribe all the topics
            broker->subscribe(ctx, get_msg_topic(tmx::messages::GPSDSampleMessage()), _worker.descriptor());
            broker->subscribe(ctx, get_msg_topic(tmx::messages::GPSDVersionMessage()), _worker.descriptor());
            broker->subscribe(ctx, get_msg_topic(tmx::messages::GPSDDeviceMessage()), _worker.descriptor());
            broker->subscribe(ctx, get_msg_topic(tmx::messages::GPSDNAVPVTMessage()), _worker.descriptor());
            broker->subscribe(ctx, get_msg_topic(tmx::messages::GPSDDecodeRawMessage()), _worker.descriptor());
            //broker->subscribe(ctx, get_msg_topic(tmx::messages::GPSDUBXConfigMessage()), _worker.descriptor());
        }

		if (broker->is_connected(ctx)) {
            struct gps_data_t *gps = tmx::common::any_cast<struct gps_data_t>(&(ctx.at(ctx.get_scheme())));
            if (gps) {
                int fd = gps->gps_fd;
                FD_SET(fd, &fds);

                nfds = std::max(nfds, fd);
            }
		}
	}

	// Look for new bytes available on any of the open streams
	r = ::select(nfds+1, &fds, NULL, NULL, &tv);
	if (r > 0) {
        for (typename UInt32::value_type i: { WATCH_JSON, WATCH_NMEA, WATCH_RAW }) {
            auto &ctx = get_connections()[(i >> 4)];
			if (!ctx) continue;

            auto broker = get_broker(ctx);
			if (broker && broker->is_connected(ctx)) {
                tmx::messages::GPSDSampleMessage msg { i };
                struct gps_data_t *gps = tmx::common::any_cast<struct gps_data_t>(&(ctx.at(ctx.get_scheme())));
				if (gps && FD_ISSET(gps->gps_fd, &fds))
			        broker->publish(ctx, get_msg_topic(msg), msg);
			}
		}
	} else if (r < 0 && errno != EINTR) {
		this->on_error(std::string("Unable to select on file descriptors: ") + strerror(errno), message::TmxMessage());
	}

}

} /* namespace GNSS */
} /* namespace v2x */
} /* namespace plugin */
} /* namespace tmx */
