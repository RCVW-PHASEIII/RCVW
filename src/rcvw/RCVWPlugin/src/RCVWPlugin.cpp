//============================================================================
// Name        : RCVWPlugin.cpp
// Author      : Battelle Memorial Institute - Greg Zink (zinkg@battelle.org)
// Version     :
// Copyright   : Battelle 2016
// Description : HRI Status Plugin - sends out a SPAT message every 10ms
//============================================================================

#define GRAVITY 9.81

//#include "HRILocation.h"
#include "RCVWParameters.hpp"

#include <tmx/common/TmxError.hpp>
#include <tmx/common/TmxLogger.hpp>
#include <tmx/common/types/Any.hpp>
#include <tmx/plugin/TmxPlugin.hpp>
#include <tmx/plugin/TmxPluginDataUpdate.hpp>
#include <tmx/plugin/utils/Clock.hpp>
#include <tmx/plugin/utils/FrequencyThrottle.hpp>
#include <tmx/plugin/utils/Uuid.hpp>
#include <tmx/plugin/utils/geo/Conversions.hpp>
#include <tmx/plugin/utils/geo/GeoVector.hpp>
#include <tmx/plugin/utils/interxn/Intersection.hpp>
#include <tmx/plugin/utils/interxn/MapSupport.hpp>
#include <tmx/plugin/utils/interxn/ParsedMap.hpp>

// J2735 message
#include <tmx/message/j2735/202007/MapData.h>
#include <tmx/message/j2735/202007/MessageFrame.h>
#include <tmx/message/j2735/202007/RoadSideAlert.h>
#include <tmx/message/j2735/202007/SPAT.h>

// V2X-Hub messages
#include <tmx/message/v2x/LocationMessageEnumTypes.hpp>
#include <tmx/message/v2x/ApplicationMessageEnumTypes.hpp>
#include <tmx/message/v2x/ApplicationMessage.hpp>
#include <tmx/message/v2x/VehicleBasicMessage.hpp>

#include <atomic>
#include <boost/algorithm/string.hpp>
#include <cmath>
#include <iostream>
#include <thread>
#include <mutex>

using namespace std;
using namespace tmx;
using namespace tmx::common;
using namespace tmx::message;
using namespace tmx::plugin::utils::geo;
using namespace tmx::plugin::utils::interxn;


namespace tmx {
namespace plugin {
namespace rcvw {
namespace app {


/**
 * <summary>
 * 	This plugin sends out the HRI Status in a SPAT Message
 * </summary>
 */

class RCVWPlugin : public tmx::plugin::TmxPlugin {
public:
    RCVWPlugin() noexcept;
    virtual ~RCVWPlugin() = default;

    TmxTypeDescriptor get_descriptor() const noexcept override;
    TmxError main() noexcept override;

    void on_message_received(const message::TmxMessage &) override;

protected:
    common::types::Array<common::types::Any> get_config_description() const noexcept override;
    void init() noexcept override;

private:
    template <typename _T>
    void get_config_value(const char *key, std::atomic<_T> &cache);

    // The message handlers
    void handle_parameter_update(TmxPluginDataUpdate const &, message::TmxMessage const &);
    void handle_map(message::TmxData const &, message::TmxMessage const &);
    void handle_spat(message::TmxData const &, message::TmxMessage const &);
    void handle_rsa(message::TmxData const &, message::TmxMessage const &);
    void handle_location(message::TmxData const &, message::TmxMessage const &);
    void handle_gnss(message::TmxData const &, message::TmxMessage const &);
    void handle_vbm(message::v2x::VehicleBasicMessage const &, message::TmxMessage const &);

private:
    //Config Values
    std::mutex _dataLock;
    std::atomic<double> _safetyOffset;
    std::atomic<double> _reactionTime;
    std::atomic<uint64_t> _messageExpiration;
    std::atomic<unsigned int> _outputInterface;
    std::atomic<bool> _mapReceived;
    std::atomic<bool> _spatReceived;
    std::atomic<double> _distanceToHRI;
    std::atomic<double> _irExtent;
    std::atomic<double> _HRIWarningThresholdSpeed;
    std::atomic<bool> _useCalculatedDeceleration;

    //Values for stopping distance calculation
    std::mutex _locationLock;
    std::atomic<double> _speed;
    std::atomic<double> _prevSpeed;
    std::atomic<double> _prevPrevSpeed;
    std::atomic<uint64_t> _speedTime;
    std::atomic<uint64_t> _prevSpeedTime;
    std::atomic<double> _speedVBM;
    std::atomic<double> _prevSpeedVBM;
    std::atomic<double> _prevPrevSpeedVBM;
    std::atomic<uint64_t> _speedTimeVBM;
    std::atomic<uint64_t> _prevSpeedTimeVBM;
    std::atomic<double> _heading;
    std::atomic<double> _acceleration;
    std::atomic<double> _horizontalDOP;
    std::atomic<double> _mu; // Coefficient of friction, should probably use kinetic friction to be conservative
    std::atomic<double> _weatherFactor;
    std::atomic<double> _lastCalculatedExpectedStopDistance;
    std::atomic<double> _lastCalculatedAcceleration;

    //Positioning Values
    std::atomic<uint64_t> _lastLocation;
    std::atomic<bool> _locationReceived;
    std::atomic<bool> _rtkReceived;
    std::atomic<bool> _locationProcessed;
    std::atomic<double> _lat;
    std::atomic<double> _long;
    std::atomic<double> _altitude;
    std::atomic<uint8_t> _rtkType;

    //Map Data
    std::atomic<uint64_t> _lastMap;
    std::shared_ptr<MapData> _mapData;

    //SPAT Data
    std::atomic<uint64_t> _lastSpat;
    std::shared_ptr<SPAT> _spatData;
    std::atomic<bool> _preemption;
    std::atomic<bool> _inLane;

    //RSA Data
    std::shared_ptr<RoadSideAlert> _rsaData;

    //VBM Data
    std::atomic<uint64_t> _lastVBM;

    //Warning Queue, lowest to highest priority
    std::atomic<bool> _availableActive;
    std::atomic<bool> _approachInformActive;
    std::atomic<bool> _approachWarningActive;
    std::atomic<bool> _hriWarningActive;
    std::atomic<bool> _errorActive;

    //other
    std::atomic<double> _lastLoggedspeed;
    std::atomic<uint64_t> _lastLocationTime;
    std::atomic<uint8_t> _stateErrorMessage;
    std::atomic<uint8_t> _changeDirectionCount;

    //V2
    std::atomic<double> _v2AntennaPlacementXMeters;  //measured from front left corner
    std::atomic<double> _v2AntennaPlacementYMeters;  //measured from front left corner
    std::atomic<double> _v2AntennaHeightMeters;
    std::atomic<double> _v2GPSErrorMeters;
    std::atomic<double> _v2ReactionTimeSec;
    std::atomic<double> _v2CommunicationLatencySec;
    std::atomic<double> _v2ApplicationLatencySec;
    std::atomic<double> _v2MinDecelerationCarMPSS;
    std::atomic<double> _v2MinDecelerationLightTruckMPSS;
    std::atomic<double> _v2MinDecelerationHeavyTruckMPSS;
    std::atomic<uint64_t> _v2vehicleType;
    std::atomic<double> _v2vehicleLength;
    std::atomic<bool> _v2useVBMDeceleration;
    std::atomic<bool> _v2LogSPAT;
    std::atomic<uint64_t> _v2CriticalMessageExpiration;
    std::atomic<bool> _v2UseConfigGrade;
    std::atomic<double> _v2Grade;
    std::atomic<bool> _v2CheckRTK;
    std::atomic<bool> _v2CheckLocationFrequency;
    std::atomic<uint64_t> _v2LocationFrequencySampleSize;
    std::atomic<double> _v2MinumumLocationFrequency;
    std::atomic<double> _v2LocationFrequencyTargetIntervalMS;
    std::atomic<double> _v2LocationFrequencyCurrentIntervalMS;
    std::atomic<uint64_t> _v2LocationFrequencyCount;
    std::atomic<double> _v2MaxHeadingChange;
    std::atomic<uint64_t> _v2MaxIgnoredPositions;

    typedef enum V2VehicleTypeEnum {
        Car = 1,
        LightTruck = 2,
        HeavyTruck = 3
    } V2VehicleType;

    typedef enum V2RTKTypeEnum {
        NA = 0,
        None = 1,
        Float = 2,
        Fixed = 3
    } V2RTKType;

    typedef enum V2StateErrorMessageEnum {
        NoError = 0,
        MAP = 1,
        SPaT = 2,
        Location = 3,
        Frequency = 4,
        RTK = 5
    } V2StateErrorMessage;

    //Helper Functions
    void CheckForErrorCondition(double lat, double lon, bool frequencyError);

    bool IsLocationInRangeOfEquippedHRI(double latitude, double longitude);

    uint64_t GetMsTimeSinceEpoch();

    bool IsDecelerating();

    bool InHRI(double lat, double lon, double speed, double heading);

    //bool HRIPreemptionActive();
    double GetDistanceToCrossing(double lat, double lon, double heading, double &grade);

    double GetStoppingDistance(double speed, double friction, double incline);

    double GetStoppingDistanceV2(double speed, double deceleration, double grade);

    void AlertVehicle();

    void AlertVehicle_2();

    void SendApplicationMessage(v2x::EventCodeTypes, v2x::Severity,
                                std::string = "", std::string = "", uint64_t = 0);

    void SendAvailable();

    void SendAvailableCleared();

    void SendApproachInform();

    void SendApproachInformCleared();

    void SendApproachWarning();

    void SendApproachWarningCleared();

    void SendHRIWarning();

    void SendHRIWarningCleared();

    void SendError(string message);

    void SendErrorCleared();

    utils::FrequencyThrottle<int> _statusThrottle;
};

struct parameter { };

/**
 * Creates an instance of the HRI Status Plugin
 *
 * @param name Name to identify the plugin instance
 */
RCVWPlugin::RCVWPlugin() noexcept {
    //Initialize Atomics
    _distanceToHRI = 480;
    _safetyOffset = 0.0;
    _messageExpiration = 2000;
    _speed = 0;
    _horizontalDOP = 0;
    _mu = 0.0;
    _weatherFactor = 1.0;
    _prevSpeed = 0.0;
    _prevPrevSpeed = 0.0;
    _lat = 0.0;
    _long = 0.0;
    _reactionTime = 1.0;
    _HRIWarningThresholdSpeed = 1.0;
    _useCalculatedDeceleration = false;
    _mapReceived = false;
    _spatReceived = false;
    _locationReceived = false;
    _rtkReceived = false;
    _locationProcessed = true;
    _preemption = false;
    _inLane = false;
    _availableActive = false;
    _approachInformActive = false;
    _approachWarningActive = false;
    _hriWarningActive = false;
    _errorActive = false;
    _lastMap = 0;
    _lastSpat = 0;
    _lastLocation = 0;
    _outputInterface = 0;
    _lastLoggedspeed = -1;
    _speedTime = 0;
    _prevSpeedTime = 0;
    _heading = 0;
    _altitude = 0;
    _acceleration = 0;
    _lastCalculatedExpectedStopDistance = 999999;
    _lastCalculatedAcceleration = 0;
    _lastLocationTime = 0;
    _rtkType = V2RTKType::NA;
    _stateErrorMessage = V2StateErrorMessage::NoError;
    _changeDirectionCount = 0;
    _speedVBM = 0;
    _prevSpeedVBM = 0;
    _prevPrevSpeedVBM = 0;
    _speedTimeVBM = 0;
    _prevSpeedTimeVBM = 0;

    _v2AntennaPlacementXMeters = 0.5;
    _v2AntennaPlacementYMeters = 2.5;
    _v2AntennaHeightMeters = 1.5;
    _v2GPSErrorMeters = 3.12;
    _v2ReactionTimeSec = 2.5;
    _v2CommunicationLatencySec = 0.3;
    _v2ApplicationLatencySec = 0.085;
    _v2MinDecelerationCarMPSS = 3.4;
    _v2MinDecelerationLightTruckMPSS = 2.148;
    _v2MinDecelerationHeavyTruckMPSS = 2.322;
    _v2vehicleType = V2VehicleType::Car;
    _v2vehicleLength = 4.8;
    _v2useVBMDeceleration = true;
    _v2LogSPAT = false;
    _v2CriticalMessageExpiration = 500;
    _v2UseConfigGrade = false;
    _v2Grade = 0;
    _v2CheckRTK = true;
    _v2CheckLocationFrequency = true;
    _v2LocationFrequencySampleSize = 10;
    _v2MinumumLocationFrequency = 8.9;
    _v2LocationFrequencyTargetIntervalMS = 1000.0 / _v2MinumumLocationFrequency;
    _v2LocationFrequencyCurrentIntervalMS = 0.0;
    _v2LocationFrequencyCount = 0;
    _v2MaxHeadingChange = 90.0;
    _v2MaxIgnoredPositions = 2;

    _statusThrottle.set_Frequency(std::chrono::seconds(2));

    // Register a handler for each of the config values
    for (auto &desc: this->get_config_description()) {
        const TmxData param { desc };
        this->register_handler<parameter>(this->get_topic("config/" + param["key"].to_string()), this,
                                          &RCVWPlugin::handle_parameter_update);
    }

    //We want to listen for Map/Spat Messages along with Location, RSA and VBM
    this->register_handler<MapData_t>("J2735/MAP", this, &RCVWPlugin::handle_map);
    this->register_handler<SPAT_t>("J2735/SPAT", this, &RCVWPlugin::handle_spat);
    this->register_handler<RoadSideAlert_t>("J2735/RSA", this, &RCVWPlugin::handle_rsa);
    this->register_handler<TmxData>("gpsd/TPV", this, &RCVWPlugin::handle_location);
    this->register_handler<v2x::VehicleBasicMessage>("V2X/VBM", this, &RCVWPlugin::handle_vbm);
}

TmxTypeDescriptor RCVWPlugin::get_descriptor() const noexcept {
    auto const &descr = TmxPlugin::get_descriptor();
    return { descr.get_instance(), typeid(*this), type_fqname(*this).data() };
}

common::types::Array<common::types::Any> RCVWPlugin::get_config_description() const noexcept {
    static TmxData description;
    if (description.is_empty()) {
        auto decoder = codec::TmxDecoder::get_decoder("json");
        if (decoder)
            decoder->decode(description.get_container(), to_byte_sequence(RCVW_PLUGIN_PARAMETERS));
    }

    return description.to_array();
}

template <typename _T>
void RCVWPlugin::get_config_value(const char *key, std::atomic<_T> &cache) {
    cache = this->get_config(key);
}

void RCVWPlugin::on_message_received(const message::TmxMessage &msg) {
    // Do not decode UPER encoded messages
    if (std::strncmp("asn.1", msg.get_encoding().c_str(), 5) == 0)
        this->invoke_handlers(msg.get_payload_string(), msg);
    else
        TmxPlugin::on_message_received(msg);
}

/**
 * Update the configuration parameters from the database
 */
void RCVWPlugin::handle_parameter_update(const tmx::plugin::TmxPluginDataUpdate &, const message::TmxMessage &) {
    // Update the entire cache
    this->get_config_value("Friction", _mu);
    this->get_config_value("Safety Offset", _safetyOffset);
    this->get_config_value("Reaction Time", _reactionTime);
    this->get_config_value("Message Expiration", _messageExpiration);
    this->get_config_value("Output Interface", _outputInterface);
    this->get_config_value("Distance To HRI", _distanceToHRI);
    this->get_config_value("Extended Intersection", _irExtent);
    this->get_config_value("HRI Warning Threshold Speed", _HRIWarningThresholdSpeed);
    this->get_config_value("Use Calculated Deceleration", _useCalculatedDeceleration);

    this->get_config_value("V2 Antenna Placement X", _v2AntennaPlacementXMeters);
    this->get_config_value("V2 Antenna Placement Y", _v2AntennaPlacementYMeters);
    this->get_config_value("V2 Antenna Height", _v2AntennaHeightMeters);
    this->get_config_value("V2 GPS Error", _v2GPSErrorMeters);
    this->get_config_value("V2 Reaction Time", _v2ReactionTimeSec);
    this->get_config_value("V2 Communication Latency", _v2CommunicationLatencySec);
    this->get_config_value("V2 Application Latency", _v2ApplicationLatencySec);
    this->get_config_value("V2 Deceleration Car", _v2MinDecelerationCarMPSS);
    this->get_config_value("V2 Deceleration Light Truck", _v2MinDecelerationLightTruckMPSS);
    this->get_config_value("V2 Deceleration Heavy Truck", _v2MinDecelerationHeavyTruckMPSS);
    this->get_config_value("V2 Vehicle Type", _v2vehicleType);
    this->get_config_value("V2 Vehicle Length", _v2vehicleLength);
    this->get_config_value("V2 Use VBM Deceleration", _v2useVBMDeceleration);
    this->get_config_value("V2 Log SPAT", _v2LogSPAT);
    this->get_config_value("V2 Critical Message Expiration", _v2CriticalMessageExpiration);
    this->get_config_value("V2 Use Config Grade", _v2UseConfigGrade);
    this->get_config_value("V2 Grade", _v2Grade);
    this->get_config_value("V2 Check RTK", _v2CheckRTK);
    this->get_config_value("V2 Check Location Frequency", _v2CheckLocationFrequency);
    this->get_config_value("V2 Location Frequency Sample Size", _v2LocationFrequencySampleSize);
    this->get_config_value("V2 Minimum Location Frequency", _v2MinumumLocationFrequency);
    this->get_config_value("V2 Max Heading Change", _v2MaxHeadingChange);
    this->get_config_value("V2 Max Ignored Positions", _v2MaxIgnoredPositions);

    _v2LocationFrequencyTargetIntervalMS = 1000.0 / _v2MinumumLocationFrequency;
    _v2LocationFrequencyCurrentIntervalMS = 0;
    _v2LocationFrequencyCount = 0;
}

/**
 * Actions to perform when the state of the plugin changes
 *
 * @param state
 */
void RCVWPlugin::init() noexcept {
    TmxPlugin::init();

    this->set_status("HRI", "Not Present");

    //this->set_statustatus("Warning", "Not Active");
    this->set_status("Map Received", false);
    this->set_status("Location Received", false);
    this->set_status("RTK Type", "");
    this->set_status("Spat Received", false);
    this->set_status("Near Active HRI", "");
}

template <typename _Tp>
TmxError do_decode(asn_TYPE_descriptor_t const *descriptor, _Tp **value, byte_sequence const &bytes) {
    auto ret = asn_decode(nullptr, ATS_UNALIGNED_BASIC_PER, descriptor,
                                      (void **)value, bytes.data(), bytes.length());
    if (ret.code == RC_OK)
        return { };

    std::string err { "ASN.1 decoding of " };
    err.append(type_fqname<_Tp>());
    err.append(" type from ");
    err.append(byte_string_encode(bytes));
    err.append(" failed after ");
    err.append(std::to_string(ret.consumed));
    err.append(" bytes.");
    return { ret.code, err };
}

template <typename _E>
static TMX_CONSTEXPR_FN auto enum_choice_name(_E val) {
    return enums::enum_name(val).substr(1 + common::type_short_name(val).length());
}

void RCVWPlugin::handle_map(TmxData const &, TmxMessage const &msg) {
    TLOG(DEBUG3) << "Enter " << TMX_PRETTY_FUNCTION << " with " << msg.to_string();

    // Decode the map
    codec::TmxCodec codec { msg };
    auto bytes = codec.get_payload_bytes();

    MessageFrame *frame = nullptr;
    auto err = do_decode(&asn_DEF_MessageFrame, &frame, to_byte_sequence(bytes.c_str(), bytes.size()));
    if (err) {
        this->broadcast<TmxError>(err, this->get_topic("error"), __FUNCTION__);
        return;
    }

    if (!frame) {
        this->broadcast<TmxError>({ ENOMEM, "Failed to create MessageFrame" }, this->get_topic("error"), __FUNCTION__);
        return;
    }

    if (frame->value.present != MessageFrame__value_PR_MapData) {
        this->broadcast<TmxError>({ EINVAL, std::string("Received invalid MessageFrame of type ") +
            enum_choice_name(frame->value.present).data() }, this->get_topic("error"), __FUNCTION__);
    }

    std::shared_ptr<MapData_t> copy;
    {
        std::lock_guard<std::mutex> lock(this->_dataLock);
        this->_mapData.reset(&frame->value.choice.MapData, [frame](auto *) {
            ASN_STRUCT_FREE(asn_DEF_MessageFrame, frame);
        });

        copy = this->_mapData;
    }

    int newIntersectionId = copy->intersections->list.array[0]->id.id;
    TLOG(DEBUG1) << "MAP Received, IntersectionID: " << newIntersectionId;

    if (!_mapReceived) {
        Intersection intersection;

        intersection.LoadMap(copy);

        WGS84Point location(_lat, _long);

        //MapSupport mapSupp;
        //MapMatchResult r = mapSupp.FindVehicleLaneForPoint(location, intersection.Map);

        if (!_mapReceived.exchange(true))
            this->set_status("Map Received", true);

        _lastMap = GetMsTimeSinceEpoch();
    } else {
        //int oldIntersectionId = _mapData.get<int>("MapData.intersections.IntersectionGeometry.id.id", -1);
        int oldIntersectionId = copy->intersections->list.array[0]->id.id;
        if (newIntersectionId == oldIntersectionId) {
            _lastMap = GetMsTimeSinceEpoch();
        }
    }
}

void RCVWPlugin::handle_spat(TmxData const &, TmxMessage const &msg) {
    TLOG(DEBUG3) << "Enter " << TMX_PRETTY_FUNCTION << " with " << msg.to_string();

    // Decode the SPAT
    codec::TmxCodec codec { msg };
    auto bytes = codec.get_payload_bytes();

    MessageFrame *frame = nullptr;
    auto err = do_decode(&asn_DEF_MessageFrame, &frame, to_byte_sequence(bytes.c_str(), bytes.size()));
    if (err) {
        this->broadcast<TmxError>(err, this->get_topic("error"), __FUNCTION__);
        return;
    }

    if (!frame) {
        this->broadcast<TmxError>({ ENOMEM, "Failed to create MessageFrame" }, this->get_topic("error"), __FUNCTION__);
        return;
    }

    if (frame->value.present != MessageFrame__value_PR_SPAT) {
        this->broadcast<TmxError>({ EINVAL, std::string("Received invalid MessageFrame of type ") +
            enum_choice_name(frame->value.present).data() }, this->get_topic("error"), __FUNCTION__);
        return;
    }

    std::shared_ptr<SPAT_t> copy;
    {
        std::lock_guard<std::mutex> lock(this->_dataLock);
        this->_spatData.reset(&frame->value.choice.SPAT, [frame](auto *) {
            ASN_STRUCT_FREE(asn_DEF_MessageFrame, frame);
        });

        copy = this->_spatData;
    }


//	if(_mapReceived)
//	{
//		MapDataMessage mapCopy;
//		{
//			std::lock_guard<mutex> lock(_dataLock);
//			mapCopy  = _mapData;
//		}
//
//		Intersection intersection;
//		if(intersection.LoadMap(mapCopy))
//		{
//			if(intersection.DoesSpatMatchMap(msg))
//			{
//				if(!_spatReceived)
//				{
//					this->set_status("Spat Received", true);
//				}
//				_spatReceived = true;
//				_lastSpat = GetMsTimeSinceEpoch();
//				std::lock_guard<mutex> lock(_dataLock);
//				_spatData = msg;
//
//			}
//		}
//
//	}


    //int spatInterId = msg.get<int>("SPAT.intersections.IntersectionState.id.id", -1);
    int spatInterId = copy->intersections.list.array[0]->id.id;
    TLOG(DEBUG1) << "SPAT Received, IntersectionID: " << spatInterId;

    if (_mapReceived) {
        //int intersectionId = _mapData.get<int>("MapData.intersections.IntersectionGeometry.id.id", -1);
        int intersectionId = copy->intersections.list.array[0]->id.id;
        if (intersectionId == spatInterId) {
            if (!_spatReceived.exchange(true))
                this->set_status("Spat Received", true);

            _lastSpat = GetMsTimeSinceEpoch();

            if (_v2LogSPAT && TmxLogger::can_log(TmxLogLevel::DEBUG)) {
                // Serialize the SPAT message to XML
                char *buffer;
                size_t bufSize;
                FILE *mStream = open_memstream(&buffer, &bufSize);
                if (mStream) {
                    if (xer_fprint(mStream, &asn_DEF_MapData, copy.get()) == 0)
                        TLOG(DEBUG) << "SPAT Received: " << buffer;

                    fclose(mStream);
                }

                free(buffer);
                buffer = nullptr;
            }
        }
    }
}

void RCVWPlugin::handle_rsa(TmxData const &, TmxMessage const &msg) {
    TLOG(DEBUG3) << "Enter " << TMX_PRETTY_FUNCTION << " with " << msg.to_string();

    // Decode the RSA
    codec::TmxCodec codec { msg };
    auto bytes = codec.get_payload_bytes();

    MessageFrame *frame = nullptr;
    auto err = do_decode(&asn_DEF_MessageFrame, &frame, to_byte_sequence(bytes.c_str(), bytes.size()));
    if (err) {
        this->broadcast<TmxError>(err, this->get_topic("error"), __FUNCTION__);
        return;
    }

    if (!frame) {
        this->broadcast<TmxError>({ ENOMEM, "Failed to create MessageFrame" }, this->get_topic("error"), __FUNCTION__);
        return;
    }

    if (frame->value.present != MessageFrame__value_PR_RoadSideAlert) {
        this->broadcast<TmxError>({ EINVAL, std::string("Received invalid MessageFrame of type ") +
            enum_choice_name(frame->value.present).data() }, this->get_topic("error"), __FUNCTION__);
        return;
    }

    std::shared_ptr<RoadSideAlert_t> copy;
    {
        std::lock_guard<std::mutex> lock(this->_dataLock);
        this->_rsaData.reset(&frame->value.choice.RoadSideAlert, [frame](auto *) {
            ASN_STRUCT_FREE(asn_DEF_MessageFrame, frame);
        });

        copy = this->_rsaData;
    }

    int itisCode = copy->typeEvent;

    TLOG(DEBUG1) << "RSA Received, ITIS code: " << itisCode;
    //rainy
    _weatherFactor = 0.6;
    //snowy
    //heavy snow
    if (itisCode >= 4866 && itisCode <= 4872) //snow
        _weatherFactor = 0.45;
    else if (itisCode >= 4875 && itisCode <= 4876) //ice
        _weatherFactor = 0.45;
    else if (itisCode >= 4881 && itisCode <= 4888) //rain
        _weatherFactor = 0.6;
    else
        _weatherFactor = 1;
}

/**
 * Handles location messages as they are received and extracts
 * the needed information.
 *
 * @param msg The decoded version of the LocationMessage.
 * @param routeableMsg the encoded version of the message routed by TMX
 */
void RCVWPlugin::handle_location(TmxData const &loc, TmxMessage const &msg) {
    TLOG(DEBUG3) << "Enter " << TMX_PRETTY_FUNCTION << " with " << msg.to_string();
    uint64_t locationInterval = 0;
    double frequency = 0.0;
    v2x::SignalQualityTypes signalQuality;

    double locHeading = loc["track"];
    double locLatitude = loc["lat"];
    double locLongitude = loc["lon"];
    double locAltitude = loc["altHAE"];
    double locSpeed_mps = loc["speed"];
    auto locSignalQuality = enums::enum_cast<v2x::SignalQualityTypes>((short)loc["status"]);

    std::string tmStr = loc["time"];
    auto locTime = utils::Clock::GetMillisecondsSinceEpoch(tmStr, "%FT%T");

    // Need to scan for the milliseconds
    auto idx = tmStr.find_last_of('.');
    if (idx < tmStr.length())
        locTime += std::atoi(tmStr.substr(idx + 1).c_str());

//    std::lock_guard<mutex> lock(_locationLock);
    string rtkType = "none";
    double heading;
    double headingChange = 0;
    double tmp_double;
    uint64_t tmp_uint64_t;

    // Get the time of the message

    //check if this is a duplicate or old position data
    if (locTime <= _lastLocationTime)
        return;
    _lastLocationTime = locTime;
    if (!_locationReceived)
        this->set_status("Location Received", true);

    _locationReceived = true;
    uint64_t currentTime = GetMsTimeSinceEpoch();
    locationInterval = locTime - _lastLocation;
    _lastLocation = locTime;

    //if its been out for more than twice the critical message timer then restart the sampling
    if (locationInterval > 2 * _v2CriticalMessageExpiration) {
        _v2LocationFrequencyCount = 0;
        _v2LocationFrequencyCurrentIntervalMS = 0.0;
    }

    //calculate running average interval between location messages
    //always have one less sampling interval than the sample size of messages
    if (_v2LocationFrequencyCount < _v2LocationFrequencySampleSize) {
        //skip calculations for first point
        if (_v2LocationFrequencyCount > 0) {
            _v2LocationFrequencyCurrentIntervalMS =
                    ((_v2LocationFrequencyCount - 1) * _v2LocationFrequencyCurrentIntervalMS) + locationInterval;
            _v2LocationFrequencyCurrentIntervalMS = _v2LocationFrequencyCurrentIntervalMS / _v2LocationFrequencyCount;
        }
        _v2LocationFrequencyCount++;
    } else {
        _v2LocationFrequencyCurrentIntervalMS =
                ((_v2LocationFrequencyCount - 2) * _v2LocationFrequencyCurrentIntervalMS) + locationInterval;
        _v2LocationFrequencyCurrentIntervalMS = _v2LocationFrequencyCurrentIntervalMS / (_v2LocationFrequencyCount - 1);
    }

    //calculate heading change
    heading = _heading;
    if (locHeading > heading)
        headingChange = locHeading - heading;
    else
        headingChange = heading - locHeading;
    //if heading changed more than (configured) degrees then throw point out, only do it (configured) in a row
    if (_changeDirectionCount < _v2MaxIgnoredPositions && headingChange > _v2MaxHeadingChange) {
        //keep previous location data except for time
        if (currentTime - _lastVBM > _v2CriticalMessageExpiration) {
            //use location data
            _prevSpeedTime.exchange(_speedTime);
            _speedTime = currentTime;
        } else {
            //use last saved VBM data
            tmp_uint64_t = _prevSpeedTimeVBM;
            _prevSpeedTime = tmp_uint64_t;
            tmp_uint64_t = _speedTimeVBM;
            _speedTime = tmp_uint64_t;
        }
        _changeDirectionCount++;
    } else {
        //use location message speed if haven't received VBM in expiration interval
        if (currentTime - _lastVBM > _v2CriticalMessageExpiration) {
            //use location data
            _prevPrevSpeed.exchange(_prevSpeed);
            _prevSpeed.exchange(_speed);
            _speed = locSpeed_mps;
            _prevSpeedTime.exchange(_speedTime);
            _speedTime = currentTime;
        } else {
            //use last saved VBM data
            tmp_double = _prevPrevSpeedVBM;
            _prevPrevSpeed = tmp_double;
            tmp_double = _prevSpeedVBM;
            _prevSpeed = tmp_double;
            tmp_double = _speedVBM;
            _speed = tmp_double;
            tmp_uint64_t = _prevSpeedTimeVBM;
            _prevSpeedTime = tmp_uint64_t;
            tmp_uint64_t = _speedTimeVBM;
            _speedTime = tmp_uint64_t;
        }
        _lat = locLatitude;
        _long = locLongitude;
        //change heading only if speed is not zero
        if (_speed != 0)
            _heading = locHeading;
        _altitude = locAltitude;
        _changeDirectionCount = 0;
    }

    //check location message for RTK fix
    signalQuality = locSignalQuality.has_value() ? locSignalQuality.value() : v2x::SignalQualityTypes::Invalid;
    if (signalQuality == v2x::SignalQualityTypes::RealTimeKinematic) {
        _rtkReceived = true;
        if (_rtkType != V2RTKType::Fixed)
            this->set_status("RTK Type", "Fixed");
        _rtkType = V2RTKType::Fixed;
        rtkType = "fixed";
    } else if (signalQuality == v2x::SignalQualityTypes::FloatRTK) {
        _rtkReceived = true;
        if (_rtkType != V2RTKType::Float)
            this->set_status("RTK Type", "Float");
        _rtkType = V2RTKType::Float;
        rtkType = "float";
    } else {
        _rtkReceived = false;
        if (_rtkType != V2RTKType::None)
            this->set_status("RTK Type", "None");
        _rtkType = V2RTKType::None;
        rtkType = "none";
    }

    _locationProcessed = false;
    if (_v2LocationFrequencyCurrentIntervalMS != 0)
        frequency = 1000.0 / _v2LocationFrequencyCurrentIntervalMS;
    else
        frequency = 0.0;
    //TLOG(DEBUG) << "LOC Id, MsgTime, CurTime: " <<  msg.get_Id() << ", " << msg.get_Time() << ", " << currentTime;
    heading = locHeading;
    TLOG(DEBUG) << std::setprecision(6) << "LOC TIME, LOC SPEED, LOC HEADING, SPEED, HEADING, RTK, FREQUENCY: "
                   << locTime << ", " << locSpeed_mps << ", " << heading << ", " << _speed << ", "
                   << _heading << ", " << rtkType << ", " << frequency;
    if (_changeDirectionCount > 0)
        TLOG(DEBUG) << "LOC change ignored count: " << (int) _changeDirectionCount;
}

/**
 * Handles location messages as they are received and extracts
 * the needed information.
 *
 * @param msg The decoded version of the LocationMessage.
 * @param routeableMsg the encoded version of the message routed by TMX
 */
void RCVWPlugin::handle_vbm(v2x::VehicleBasicMessage const &vbm, TmxMessage const &msg) {
//    std::lock_guard<mutex> lock(_locationLock);
    uint64_t currentTime = GetMsTimeSinceEpoch();
    _lastVBM = currentTime;
    _prevPrevSpeedVBM.exchange(_prevSpeedVBM);
    _prevSpeedVBM.exchange(_speedVBM);
    _speedVBM = vbm.get_Speed_mps();
    _prevSpeedTimeVBM.exchange(_speedTimeVBM);
    _speedTimeVBM = currentTime;
    _acceleration = vbm.get_Acceleration();
    TLOG(DEBUG) << std::setprecision(10) << "VBM SPEED, VBM ACCELERATION: " << _speedVBM << ", " << _acceleration;
}

/**
 * Function determines if the vehicle is in a HRI or not
 * based on the data acquired from the map message and the
 * current vehicle location.
 *
 * @return true if the vehicle is currently in the HRI, false otherwise.
 */
bool RCVWPlugin::InHRI(double lat, double lon, double speed, double heading) {
    WGS84Point front;
    WGS84Point back;
    double backwardsHeading;

    std::shared_ptr<MapData_t> mapCopy;
    {
        std::lock_guard<mutex> lock(_dataLock);
        mapCopy = _mapData;
    }

    Intersection intersection;
    if (!intersection.LoadMap(mapCopy)) {
        TLOG(ERR) << "Problem reading in the current map message.";
        return false;
    }

    WGS84Point location(lat, lon);

    MapSupport mapSupp;
    mapSupp.SetExtendedIntersectionPercentage(_irExtent);

    MapMatchResult r = mapSupp.FindVehicleLaneForPoint(location, intersection.Map);

    if (r.LaneNumber == 0) {
        return true;
    }

    //check front and back of vehicle
    //calculate front and back points
    front = GeoVector::DestinationPoint(location, heading, _v2AntennaPlacementYMeters);
    backwardsHeading = heading + 180.0;
    if (backwardsHeading >= 360.0)
        backwardsHeading -= 360.0;
    back = GeoVector::DestinationPoint(location, backwardsHeading, _v2vehicleLength - _v2AntennaPlacementYMeters);
    //check points
    r = mapSupp.FindVehicleLaneForPoint(front, intersection.Map);
    if (r.LaneNumber == 0) {
        return true;
    }
    r = mapSupp.FindVehicleLaneForPoint(back, intersection.Map);
    if (r.LaneNumber == 0) {
        return true;
    }

    return false;
}

/**
 * Determine if any lane has preemption active
 * ***THIS FUNCTION IS NOT COMPLETE***
 */
//bool RCVWPlugin::HRIPreemptionActive()
//{
//	MapDataMessage mapCopy;
//	SpatMessage spatCopy;
//	{
//		std::lock_guard<mutex> lock(_dataLock);
//		mapCopy = _mapData;
//		spatCopy = _spatData;
//	}
//
//	Intersection intersection;
//	if(!intersection.LoadMap(mapCopy))
//	{
//		TLOG(ERR) << "Problem reading in the current map message.";
//		return false;
//	}
//
//	// Check to see if SPAT and MAP intersection Ids match.
//	if(!intersection.DoesSpatMatchMap(spatCopy))
//	{
//		return -1;
//	}
//
//	for (std::list<MapLane>::iterator i = intersection.Map.Lanes.begin();
//				i != intersection.Map.Lanes.end(); i++)
//	{
//
//		MapSupport mapSupp;
//		int signalGroup = mapSupp.GetSignalGroupForVehicleLane(i->LaneNumber, intersection.Map);
//
//		std::string spatSeg = "";
//		//Check all three types of lanes that stand for vehicle lanes.
//		if (i->Type == Vehicle || i->Type == Computed || i->Type == Egress)
//		{
//			if(!intersection.IsSignalForGroupRedLight(spatCopy, signalGroup))
//			{
//			}
//		}
//
//	}
//}

/**
 * Function finds the distance to the crossing based on the current
 * location and the data in the Map message
 *
 * @return distance to the crossing in meters, -1 indicates that the vehicle is not in a lane.
 */
double RCVWPlugin::GetDistanceToCrossing(double lat, double lon, double heading, double &grade) {
    std::shared_ptr<MapData_t> mapCopy;
    std::shared_ptr<SPAT_t> spatCopy;
    {
        std::lock_guard<mutex> lock(_dataLock);
        mapCopy = _mapData;
        spatCopy = _spatData;
    }

    Intersection intersection;

    intersection.LoadMap(mapCopy);

    WGS84Point location(lat, lon);

    MapSupport mapSupp;

    MapMatchResult r = mapSupp.FindVehicleLaneForPoint(location, heading, intersection.Map);
    //check if not in map
    if (r.LaneNumber == -1) {
        //not in lane or map
        if (_preemption)
            this->set_status("HRI", "Not Present");
        _preemption = false;
        _inLane = false;
        return -1;
    }
    if (r.LaneNumber > 0) {
        _inLane = true;
    } else {
        _inLane = false;
    }

    // Check to see if SPAT and MAP intersection Ids match.
    if (!intersection.DoesSpatMatchMap(*spatCopy)) {
        return -1;
    }

    int signalGroup = mapSupp.GetSignalGroupForVehicleLane(r.LaneNumber, intersection.Map);

    TLOG(DEBUG) << "Lane, SignalGroup = " << r.LaneNumber << ", " << signalGroup;
    std::string spatSeg = "";

    if (!intersection.IsSignalForGroupRedLight(*spatCopy, signalGroup)) {
        if (_preemption)
            this->set_status("HRI", "Not Present");
        _preemption = false;
    } else {
        if (!_preemption)
            this->set_status("HRI", "Present");
        _preemption = true;
    }

    int laneSegment = r.LaneSegment;

    if ((r.IsInLane == false && r.IsNearLane == false) || r.LaneNumber < 0 || laneSegment < 1 || r.IsEgress) {
        _inLane = false;
        return -1;
    }

    if (_v2UseConfigGrade) {
        grade = _v2Grade;
    } else {
        grade = r.Grade;
    }
    TLOG(DEBUG) << "IsInLane: " << r.IsInLane << ", IsNearLane: " << r.IsNearLane << ", LaneNumber: " << r.LaneNumber
                   << ", LaneSegment: " << r.LaneSegment << ", Grade: " << grade;

    //!!!!!!!!!!!!!!!!!!!!!!
    //FOR TESTING ONLY
    //!!!!!!!!!!!!!!!!!!!!!!
    //_preemption = true;


    // Calculate the distance to the crossing on a node by node basis
    // to account for curves when approaching the intersection.

    double distance = 0.0;
    int nodeIndex = 0;
    WGS84Point point1;
    WGS84Point point2;
    std::list<LaneNode>::iterator it;
    std::list<LaneNode> t_nodes = intersection.GetLaneNodes(*mapCopy, r.LaneNumber, 0.0, 0.0);
    for (it = t_nodes.begin(); it != t_nodes.end() && nodeIndex < laneSegment; ++it) {
        point1 = point2;
        point2 = it->Point;
        TLOG(DEBUG1) << "node: " << point2.Latitude << ", " << point2.Longitude;
        if (nodeIndex > 0) {
            //TLOG(DEBUG1) << "point1: " << point1.Latitude << ", " << point1.Longitude;
            //TLOG(DEBUG1) << "point2: " << point2.Latitude << ", " << point2.Longitude;
            distance += Conversions::DistanceMeters(point1, point2);
            //TLOG(DEBUG1) << "distance: " << distance;
        }
        nodeIndex++;
    }
    distance += Conversions::DistanceMeters(point2, location);
    TLOG(DEBUG1) << "final distance: " << distance;

//the code below causes seg fault, not sure why, about the same as code above that replaces it
//	auto node = intersection.GetLaneNodes(mapCopy, r.LaneNumber, 0.0, 0.0).begin();
//
//	for(int index = 0; index < nextNode; ++index)
//	{
//		WGS84Point point1 = node->Point;
//		TLOG(DEBUG1) << "point1: " << point1.Latitude << ", " << point1.Longitude;
//		node++;
//		WGS84Point point2 = node->Point;
//		TLOG(DEBUG1) << "point2: " << point2.Latitude << ", " << point2.Longitude;
//		distance += Conversions::DistanceMeters(point1, point2);
//		TLOG(DEBUG1) << "distance: " << distance;
//	}
//
//	distance += Conversions::DistanceMeters(node->Point, location);
//	TLOG(DEBUG1) << "final distance: " << distance;

    return distance;
}

/**
 * Function to calculate the stopping distance needed based on
 * the current speed of the vehicle and the coefficient of
 * friction between the tires and road for the current vehicle.
 * There are a couple of assumptions that simplify the calculation
 * of stopping distance. By including a safety offset to error on the side
 * of caution we can ignore the fact that friction is not constant in vehicle
 * dynamics.
 *
 * @param speed The current speed of the vehicle in m/s
 * @param friction The coefficient of friction between the tires and the road surface for the current vehicle
 * @param incline the slope of the road in degrees, positive is incline negative is decline
 *
 * @return The distance needed to stop in meters
 */
double RCVWPlugin::GetStoppingDistance(double speed, double friction, double incline) {
    double distance = 0.0;
    distance = (_reactionTime * speed) + ((speed * speed) / (2 * 9.8 * ((friction * cos(incline)) + sin(incline))));
    return distance;
}

/**
 * Function to calculate the stopping distance needed based on
 * the current speed of the vehicle.
 *
 * @param speed The current speed of the vehicle in m/s
 * @param deceleration The min deceleration for this vehicle in m/s^2
 * @param grade rise/run
 *
 * @return The distance needed to stop in meters
 */
double RCVWPlugin::GetStoppingDistanceV2(double speed, double deceleration, double grade) {
    double distance = 0.0;
    //if the vehicle is not moving (speed is zero which it should be with speed clamping) then return zero
    if (speed == 0.0)
        return distance;
    double v = speed * 3.6; //change to kph for formula
    double t = _v2ReactionTimeSec + _v2CommunicationLatencySec + _v2ApplicationLatencySec;
    distance = _v2AntennaPlacementYMeters + _v2GPSErrorMeters + (0.278 * v * t) +
               ((v * v) / (254 * ((deceleration / 9.81) + grade)));
    return distance;
}


/**
 * Function determines if the vehicle is decelerating based on
 * the data received from the location messages.
 *
 * @return true if decelerating, false otherwise
 */
bool RCVWPlugin::IsDecelerating() {
    if (_speed < _prevSpeed &&
        _speed < _prevPrevSpeed) // current speed is compared against two previous speeds to avoid false positives.
    {
        return true;
    }

    return false;
}

void RCVWPlugin::SendApplicationMessage(v2x::EventCodeTypes eventCode, v2x::Severity sev, std::string txt,
                                        std::string interaction, uint64_t time) {
    typedef v2x::ApplicationMessage app_t;
    static const auto _id = app_t::Id_s;

    TmxMessage timer;
    timer.set_timepoint();

    TmxData msg;
    msg["header"]["subtype"] = std::string("Application");
    msg["payload"][app_t::Id_attr::name] = utils::Uuid::NewGuid();
    msg["payload"][app_t::AppId_attr::name] = std::to_string(enums::enum_integer(v2x::ApplicationTypes::RCVW));
    msg["payload"][app_t::EventCode_attr::name] = std::to_string(enums::enum_integer(eventCode));
    msg["payload"][app_t::Severity_attr::name] = std::to_string(enums::enum_integer(sev));
    msg["payload"][app_t::CustomText_attr::name] = std::string(txt);
    if (!interaction.empty())
        msg["payload"][app_t::InteractionId_attr::name] = std::string(interaction);
    if (time > 0)
        msg["payload"][app_t::Timestamp_attr::name] = to_string(time);
    else
        msg["payload"][app_t::Timestamp_attr::name] = to_string(timer.get_timestamp());


    // Customize the message for use in the UI
    codec::TmxCodec codec;
    codec.encode(msg.get_container(), "json");
    codec.get_message().set_timepoint();
    codec.get_message().set_topic("V2X/Application");

    this->broadcast(codec.get_message());
}

/*
 * Send messages to UI
 */


void RCVWPlugin::SendAvailable() {
    TLOG(DEBUG) << "Sending Application Message: Available";
    this->set_status("Available", "Active");
    SendApplicationMessage(v2x::EventCodeTypes::RCVW2Available, v2x::Severity::Inform);
}

void RCVWPlugin::SendAvailableCleared() {
    TLOG(DEBUG) << "Sending Application Message: Clear Available";
    this->set_status("Available", "");
    SendApplicationMessage(v2x::EventCodeTypes::RCVW2Available, v2x::Severity::Info);
}

void RCVWPlugin::SendApproachInform() {
    TLOG(DEBUG) << "Sending Application Message: ApproachInform";
    this->set_status("ApproachInform", "Active");
    SendApplicationMessage(v2x::EventCodeTypes::RCVW2ApproachInform, v2x::Severity::Inform);
}

void RCVWPlugin::SendApproachInformCleared() {
    TLOG(DEBUG) << "Sending Application Message: Clear ApproachInform";
    this->set_status("ApproachInform", "");
    SendApplicationMessage(v2x::EventCodeTypes::RCVW2ApproachInform, v2x::Severity::Info);
}

void RCVWPlugin::SendApproachWarning() {
    TLOG(DEBUG) << "Sending Application Message: ApproachWarning";
    this->set_status("ApproachWarning", "Active");
    SendApplicationMessage(v2x::EventCodeTypes::RCVW2ApproachWarning, v2x::Severity::Inform);
}

void RCVWPlugin::SendApproachWarningCleared() {
    TLOG(DEBUG) << "Sending Application Message: Clear ApproachWarning";
    this->set_status("ApproachWarning", "");
    SendApplicationMessage(v2x::EventCodeTypes::RCVW2ApproachWarning, v2x::Severity::Info);
}

void RCVWPlugin::SendHRIWarning() {
    TLOG(DEBUG) << "Sending Application Message: HRIWarning";
    this->set_status("HRIWarning", "Active");

    // If the pre-emption signal is active, this is a major warning. Otherwise, it is an inform.
    SendApplicationMessage(v2x::EventCodeTypes::RCVW2HRIWarning,
                           _preemption ? v2x::Severity::Warning : v2x::Severity::Inform);
}

void RCVWPlugin::SendHRIWarningCleared() {
    TLOG(DEBUG) << "Sending Application Message: Clear HRIWarning";
    this->set_status("HRIWarning", "");
    SendApplicationMessage(v2x::EventCodeTypes::RCVW2HRIWarning, v2x::Severity::Info);
}

void RCVWPlugin::SendError(string message) {
    TLOG(DEBUG) << "Sending Application Message: Error: " << message;
    this->set_status("Error", "Active: " + message);
    SendApplicationMessage(v2x::EventCodeTypes::RCVW2Error, v2x::Severity::Inform, message);
}

void RCVWPlugin::SendErrorCleared() {
    TLOG(DEBUG) << "Sending Application Message: Clear Error";
    this->set_status("Error", "");
    SendApplicationMessage(v2x::EventCodeTypes::RCVW2Error, v2x::Severity::Info);
}


void RCVWPlugin::AlertVehicle_2() {
    bool logCalculations = false;
    bool checkDeceleration = false;
    double speed = _speed;
    double prevSpeed = _prevSpeed;
    uint64_t speedTime = _speedTime;
    uint64_t prevSpeedTime = _prevSpeedTime;
    double lat = _lat;
    double lon = _long;
    double hdop = _horizontalDOP;
    double grade = 0;
    float heading = _heading;
    bool inHRI = false;
    uint64_t lastVBM = _lastVBM;
    uint64_t v2CriticalMessageExpiration = _v2CriticalMessageExpiration;;
    bool locationProcessed = _locationProcessed.exchange(true);

    //if we have already processed this location then skip processing
    if (locationProcessed)
        return;

    uint64_t currentTime = GetMsTimeSinceEpoch();

    //calculate crossing distance, safe stopping distance, and set preemption
    //crossing distance = -1 if not in a lane

    double crossingDistance = GetDistanceToCrossing(lat, lon, heading, grade);

    //log data and calculations only if vehicle is not stopped (with location plugin latching we should get a zero speed)
    //log the data after the GetDistanceToCrossing call because _preemption is set there
    if (_lastLoggedspeed > 0 || speed > 0) {
        TLOG(DEBUG) << std::setprecision(10) << "Latitude: " << lat << ", Longitude: " << lon << ", Speed: " << speed
                       << ", PrevSpeed: " << prevSpeed << ", HDOP: " << hdop << ", Preemption: " << _preemption;
        double lastLoggedspeed = speed;
        _lastLoggedspeed = lastLoggedspeed;
        logCalculations = true;
    }

    double mu = _mu * _weatherFactor;
    double safetyStopDistanceV1 = GetStoppingDistance(speed, mu, 0.0) * _safetyOffset;
    double safetyStopDistance;
    if (_v2vehicleType == V2VehicleType::Car)
        safetyStopDistance = GetStoppingDistanceV2(speed, _v2MinDecelerationCarMPSS, grade);
    else if (_v2vehicleType == V2VehicleType::LightTruck)
        safetyStopDistance = GetStoppingDistanceV2(speed, _v2MinDecelerationLightTruckMPSS, grade);
    else if (_v2vehicleType == V2VehicleType::HeavyTruck)
        safetyStopDistance = GetStoppingDistanceV2(speed, _v2MinDecelerationHeavyTruckMPSS, grade);
    else
        safetyStopDistance = GetStoppingDistanceV2(speed, _v2MinDecelerationCarMPSS, grade);

    double expectedStopDistance = 0;
    double acceleration;

    //calculate acceleration
    if (!locationProcessed) {
        //have new data
        if (speed < prevSpeed) {
            if (_useCalculatedDeceleration)
                checkDeceleration = true;
            //calculate expected stop distance due to deceleration
            acceleration = (speed - prevSpeed) / (((double) (speedTime - prevSpeedTime)) / 1000);
            expectedStopDistance = (-1 * (speed * speed)) / (2 * acceleration);
            _lastCalculatedAcceleration = acceleration;
            _lastCalculatedExpectedStopDistance = expectedStopDistance;
            TLOG(DEBUG) << std::setprecision(10) << "Calculated Acceleration: " << acceleration
                           << ", expectedStopDistance: " << expectedStopDistance;
        } else {
            _lastCalculatedAcceleration = 0;
            _lastCalculatedExpectedStopDistance = 999999;
        }
    } else {
        //reuse old data
        if (_lastCalculatedAcceleration < 0) {
            if (_useCalculatedDeceleration)
                checkDeceleration = true;
            acceleration = _lastCalculatedAcceleration;
            expectedStopDistance = _lastCalculatedExpectedStopDistance;
            TLOG(DEBUG) << std::setprecision(10) << "Calculated Acceleration: " << acceleration
                           << ", expectedStopDistance: " << expectedStopDistance;
        }
    }

    //check for valid deceleration
    if (currentTime - lastVBM <= v2CriticalMessageExpiration && _acceleration < 0) {
        if (_v2useVBMDeceleration)
            checkDeceleration = true;
        //calculate expected stop distance due to deceleration
        expectedStopDistance = (-1 * (speed * speed)) / (2 * _acceleration);
        TLOG(DEBUG) << std::setprecision(10) << "VBM Acceleration: " << _acceleration << ", expectedStopDistance: "
                       << expectedStopDistance;
    }

    if (logCalculations) {
        //TLOG(DEBUG) << "VSE: " << _mu << ", WeatherFactor: " << _weatherFactor <<  ", ReactionTime: " << _reactionTime;
        TLOG(DEBUG) << std::setprecision(10) << "CrossingDistance: " << crossingDistance
                       << ", SafetyStopDistanceV1: " << safetyStopDistanceV1 << ", SafetyStopDistance: "
                       << safetyStopDistance << ", ExpectedStopDistance: " << expectedStopDistance;
    }


    inHRI = InHRI(lat, lon, speed, heading);

    if (!_availableActive) {
        if (_inLane || inHRI) {
            _availableActive = true;
            SendAvailable();
        }
    } else {
        if (!_inLane && !inHRI) {
            _availableActive = false;
            SendAvailableCleared();
        }
    }

    if (!_approachInformActive) {
        if (_preemption && !inHRI) {
            _approachInformActive = true;
            SendApproachInform();
        }
    } else {
        if (!_preemption || inHRI) {
            _approachInformActive = false;
            SendApproachInformCleared();
        }
    }

    if (!_approachWarningActive) {
        if (_preemption && !inHRI &&
            crossingDistance < safetyStopDistance &&
            (!checkDeceleration || expectedStopDistance > crossingDistance)) {
            _approachWarningActive = true;
            SendApproachWarning();
        }
    } else {
        if (!_preemption || inHRI ||
            crossingDistance >= safetyStopDistance ||
            (checkDeceleration && expectedStopDistance <= crossingDistance)) {
            _approachWarningActive = false;
            SendApproachWarningCleared();
        }
    }


    if (!_hriWarningActive) {
        if (inHRI && speed <= _HRIWarningThresholdSpeed) {
            _hriWarningActive = true;
            SendHRIWarning();
        }
    } else {
        if (!inHRI || speed > _HRIWarningThresholdSpeed) {
            _hriWarningActive = false;
            SendHRIWarningCleared();
        }
    }


}


/**
 * Generates a time to use for timestamps.
 *
 * @return integer timestamp in ms
 */
uint64_t RCVWPlugin::GetMsTimeSinceEpoch() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (uint64_t) ((double) (tv.tv_sec) * 1000
                       + (double) (tv.tv_usec) / 1000);
}

bool RCVWPlugin::IsLocationInRangeOfEquippedHRI(double latitude, double longitude) {
    auto locations = this->get_config("HRI Locations");
    for (std::size_t i = 0; i < (std::size_t)locations; i++) {
        double distanceToHRI = Conversions::DistanceMeters(latitude, longitude,
                                                           locations[i]["Latitude"], locations[i]["Longitude"]);

        if (distanceToHRI <= _distanceToHRI && this->_statusThrottle.Monitor(1)) {
            this->set_status("Near Active HRI", locations[i]["HRIName"].to_string().c_str());
            return true;
        }
    }

    static types::Any noHRI = types::Null();

    if (this->_statusThrottle.Monitor(1))
        this->set_status("Near Active HRI", noHRI);
    return false;
}

/**
 * Plugin Logic to run after initialization
 *
 * @return normal exit code
 */
TmxError RCVWPlugin::main() noexcept {
    uint64_t currentTime;
    double bestNextLocationInterval;
    double testLocationInterval;
    bool frequencyError = false;

    TLOG(INFO) << "Starting Plugin";

    while (this->is_running()) {
        frequencyError = false;
        //if we have at least 3 samples we have an average interval
        if (_v2CheckLocationFrequency && _v2LocationFrequencyCount > 2) {
            //test if interval out of range
            if (_v2LocationFrequencyCurrentIntervalMS > _v2LocationFrequencyTargetIntervalMS)
                frequencyError = true;
        }
        if (!_mapReceived || !_spatReceived || !_locationReceived || (_v2CheckRTK && !_rtkReceived) || frequencyError) {
            CheckForErrorCondition(_lat, _long, frequencyError);
            usleep(100000);
            continue;
        } else {
            //No longer in Error Condition if we get here, cancel any errors and move on
            if (_errorActive) {
                SendErrorCleared();
                _errorActive = false;
            }
        }

        uint64_t curTime = GetMsTimeSinceEpoch();
        bool messageCheck = false;

        if (curTime - _lastSpat > _v2CriticalMessageExpiration) {
            if (_spatReceived) {
                this->set_status("Spat Received", false);
                _spatReceived = false;
            }
            messageCheck = true;
        }

        if (curTime - _lastMap > _messageExpiration) {
            if (_mapReceived) {
                this->set_status("Map Received", false);
                _mapReceived = false;
            }
            messageCheck = true;
        }

        if (curTime - _lastLocation > _v2CriticalMessageExpiration) {
            if (_locationReceived) {
                this->set_status("Location Received", false);
                this->set_status("RTK Type", "");
                _locationReceived = false;
            }
            messageCheck = true;
        }

        //If any of the messages have expired skip the other calculations.
        if (messageCheck) {
            //usleep(500000);
            continue;
        }

        AlertVehicle_2();
        usleep(10000);
    }


    return 0;
}


void RCVWPlugin::CheckForErrorCondition(double lat, double lon, bool frequencyError) {
    bool isInRangeOfHRI = IsLocationInRangeOfEquippedHRI(lat, lon);

    bool isErrorCondition = false;
    string errorMessage = "";

    //Check for error condition
    if (isInRangeOfHRI && !_mapReceived) {
        //no MAP and in range of HRI
        isErrorCondition = true;
        errorMessage = "MAP Data Not Received";
        if (_stateErrorMessage != V2StateErrorMessage::MAP) {
            _stateErrorMessage = V2StateErrorMessage::MAP;
            this->set_status("Error", errorMessage);
        }
    } else if (isInRangeOfHRI && !_spatReceived) {
        //no SPAT and in range of HRI
        isErrorCondition = true;
        errorMessage = "SPAT Data Not Received";
        if (_stateErrorMessage != V2StateErrorMessage::SPaT) {
            _stateErrorMessage = V2StateErrorMessage::SPaT;
            this->set_status("Error", errorMessage);
        }
    } else if (!_locationReceived) {
        //no location (GPS)
        isErrorCondition = true;
        errorMessage = "Location Data Not Received";
        if (_stateErrorMessage != V2StateErrorMessage::Location) {
            _stateErrorMessage = V2StateErrorMessage::Location;
            this->set_status("Error", errorMessage);
        }
    } else if (frequencyError) {
        //GPS frequency less than target
        isErrorCondition = true;
        errorMessage = "Location Data Frequency Too Low";
        if (_stateErrorMessage != V2StateErrorMessage::Frequency) {
            _stateErrorMessage = V2StateErrorMessage::Frequency;
            this->set_status("Error", errorMessage);
        }
    } else if (_v2CheckRTK && isInRangeOfHRI && !_rtkReceived) {
        //have location but no RTK fix and in range of HRI
        isErrorCondition = true;
        errorMessage = "RTK Data Not Received";
        if (_stateErrorMessage != V2StateErrorMessage::RTK) {
            _stateErrorMessage = V2StateErrorMessage::RTK;
            this->set_status("Error", errorMessage);
        }
    } else if (!isInRangeOfHRI) {
        //clear error condition if out of range of HRI and it's not a location (GPS) error
        isErrorCondition = false;
    }

    if (isErrorCondition && !_errorActive) {
        //send error and clear all other conditions
        SendError(errorMessage);
        _errorActive = true;
        if (_availableActive) {
            _availableActive = false;
            SendAvailableCleared();
        }
        if (_approachInformActive) {
            _approachInformActive = false;
            SendApproachInformCleared();
        }
        if (_approachWarningActive) {
            _approachWarningActive = false;
            SendApproachWarningCleared();
        }
        if (_hriWarningActive) {
            _hriWarningActive = false;
            SendHRIWarningCleared();
        }
    }

    if (!isErrorCondition && _errorActive) {
        _stateErrorMessage = V2StateErrorMessage::NoError;
        SendErrorCleared();
        _errorActive = false;
    }
}

} /* namespace tmx */
} /* namespace plugin */
} /* namespace rcvw */
} /* namespace app */

int main(int argc, char *argv[])
{
    tmx::plugin::rcvw::app::RCVWPlugin plugin;
	return tmx::plugin::run(plugin, argc, argv);
}
