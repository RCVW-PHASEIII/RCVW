//============================================================================
// Name        : MmitssRSPlugin.cpp
// Author      : Battelle Memorial Institute - Ben Paselsky (paselsky@battelle.org)
// Version     :
// Copyright   : Copyright (c) 2018 Battelle Memorial Institute. All rights reserved.
// Description : Plugin that listens for requests from vehicles to modify traffic
//				 signal behavior
//============================================================================

#include <atomic>
#include <thread>
#include <queue>
#include <boost/regex.hpp>
#include <boost/lockfree/spsc_queue.hpp>
#include <unordered_map>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/foreach.hpp>
#include <boost/format.hpp>

#include "PluginClient.h"
#include <tmx/messages/message_document.hpp>
#include <ApplicationMessageEnumTypes.h>
#include <ApplicationMessage.h>
#include <GeoVector.h>
#include <MapSupport.h>
#include <TmxMessageManager.h>
#include <tmx/j2735_messages/J2735MessageFactory.hpp>
#include <Intersection.h>

#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>

using namespace std;
using namespace tmx;
using namespace tmx::utils;
using namespace tmx::messages;
using namespace tmx::messages::appmessage;
using namespace boost::property_tree;

namespace MmitssRSPlugin
{

struct SignalGroupPolicyDataStruct
{
	int32_t callPhase;
	int32_t extendPhase;
	uint64_t maxExtendTimeMS;
	uint64_t lastCallTimeMS;
};

struct PreemptionPolicyDataStruct
{
	int32_t preemptionAmbulance;
	int32_t preemptionFire;
	int32_t preemptionPolice;
};
struct VehicleDataStruct
{
	uint64_t lastBSMTimeMS{0};
	uint64_t lastSRMTimeMS{0};
	std::set<uint32_t> signalGroupCalls;
	bool extendActive{false};
	uint32_t extendSignalGroup{0};
	bool useMaxExtend{false};
	bool preemptRequested{false};
	bool preemptActivated{false};
	uint64_t preemptActivationTimeMS{0};
	uint32_t preemptRole;
	int32_t preemptPolicy;
};

/**
 * This plugin listens for BSM and SRM messages and generates traffic controller requests
 */
class MmitssRSPlugin: public TmxMessageManager
{
public:
	MmitssRSPlugin(std::string);
	virtual ~MmitssRSPlugin();
	int Main();
protected:
	void UpdateConfigSettings();
	// Virtual method overrides.
	void OnConfigChanged(const char *key, const char *value);
	void OnStateChange(IvpPluginState state);
	uint64_t GetMsTimeSinceEpoch();
	void SendApplicationMessage(ApplicationTypes applicationType, EventCodeTypes eventCode, Severity sev, std::string txt = "", std::string interaction = "", uint64_t time = 0);

	void OnMessageReceived(IvpMessage *msg);
	void OnMessageReceived(const tmx::routeable_message &routeableMsg);
	void CheckCall(double lat, double lon, double speedMetersPS, double headingDegrees, uint32_t vehicleId);
	void CheckExtend(double lat, double lon, double speedMetersPS, double headingDegrees, uint32_t vehicleId, bool useMaxExtend);
	void CallPhase(int32_t phase);
	void ClearCall(int32_t phase);
	void ActivateExtend(int32_t phase, uint64_t extendMaxEndTimestampMS);
	void CancelExtend(int32_t phase, bool force = false);
	void CancelAllActiveExtends();
	void ActivatePreempt(int32_t policy);
	void CancelPreempt(int32_t policy);

	void SNMPOpenSession();
	void SNMPCloseSession();

	bool SNMPSet(string targetOid, int value);
	bool SNMPSet(string targetOid, u_char type, const void *value, size_t len);

	bool SNMPGet(string targetOid, bool walk = false);

private:
	// locks
	mutex _configDataLock;

	//configs
	std::atomic<uint64_t> _sleepMS{100};
	string _ntcipAddress{""};
	std::atomic<uint32_t> _ntcipPort{0};
	string _signalGroupPolicyJSON{""};
	string _preemptionPolicyJSON{""};
	std::atomic<uint64_t> _bsmTimeoutMS{5000};
	std::atomic<uint64_t> _srmTimeoutMS{5000};
	std::atomic<uint64_t> _phaseControlGroupCount{2};
	std::atomic<uint64_t> _phaseCallDurationMS{3000};
	std::atomic<uint64_t> _preemptCount{10};
	std::atomic<bool> _clearPreemptsOnStartup{true};

	//config copies
	string _ntcipAddressCopy{""};
	uint32_t _ntcipPortCopy{0};
	string _signalGroupPolicyJSONCopy{""};
	string _preemptionPolicyJSONCopy{""};

	std::atomic<bool> _configChanged{false};
	bool _initialized{false};

	//call data
	double _minVehicleSpeedMetersPS{2};
	uint8_t *_currentVehicleCallGroups{NULL}; //array of phase groups for vehicle calls, one bit per phase
	bool *_currentVehicleCalls{NULL}; //array of phases
	uint64_t *_currentVehicleLastCallTimesMS{NULL}; //array of last call time for phase
	bool _callsActive{false};

	//extend data
	uint8_t *_currentVehicleExtendGroups{NULL}; //array of phase groups for vehicle extends, one bit per phase
	int32_t *_currentVehicleExtendCounts{NULL}; //array of phases
	uint64_t *_currentVehicleExtendMaxEndTimestampMS{NULL}; //array of max end timestamps for phase
	bool _extendsActive{false};


	//Map Data
	std::atomic<bool> _haveMap{false};
	Intersection *_activeIntersection{NULL};

	//SPAT Data
	std::atomic<uint64_t> _lastSpat{0};
	std::atomic<bool> _haveSpat{false};

	//BSM processing
	std::atomic<uint64_t> _lastVehicleCleanupIntervalMS{20000};
	std::atomic<uint64_t> _lastVehicleCleanupMS{0};

	//signal group info
	std::map<uint32_t, SignalGroupPolicyDataStruct> _signalGroupPolicy;
	std::atomic<bool> _haveSignalGroupPolicy{false};

	//vehicle info
	std::map<uint32_t, VehicleDataStruct> _vehicles;

	//call limits
	uint64_t _callMaxFrequencyMS{4000};
	uint64_t _callMaxTimeToIntersectionMS{20000};

	//preemption info
	std::map<uint32_t, PreemptionPolicyDataStruct> _preemptionPolicy;
	std::atomic<bool> _havePreemptionPolicy{false};
	std::map<uint32_t, int32_t> _acticvePreempts; // <vehicleID, policy>

	//snmp
	struct snmp_session _session_info;
	struct snmp_session *_session{NULL};
	bool _snmpSessionOpen{false};
	bool _snmpDestinationChanged{false};

};

static J2735MessageFactory factory;


/**
 * Construct a new MmitssRSPlugin with the given name.
 *
 * @param name The name to give the plugin for identification purposes
 */
MmitssRSPlugin::MmitssRSPlugin(string name) : TmxMessageManager(name)
{
	//init the snmp library
	init_snmp("MmitssRSPlugin");

	// Listen for all J2735 message types
	AddMessageFilter(tmx::messages::api::MSGSUBTYPE_J2735_STRING, tmx::messages::api::MSGSUBTYPE_BASICSAFETYMESSAGE_STRING);
	AddMessageFilter(tmx::messages::api::MSGSUBTYPE_J2735_STRING, tmx::messages::api::MSGSUBTYPE_MAPDATA_STRING);
	AddMessageFilter(tmx::messages::api::MSGSUBTYPE_J2735_STRING, tmx::messages::api::MSGSUBTYPE_SIGNALPHASEANDTIMINGMESSAGE_STRING);
	AddMessageFilter(tmx::messages::api::MSGSUBTYPE_J2735_STRING, tmx::messages::api::MSGSUBTYPE_SIGNALREQUESTMESSAGE_STRING);

	// Subscribe to all messages specified by the filters above.
	SubscribeToMessages();
}

MmitssRSPlugin::~MmitssRSPlugin()
{
}

void MmitssRSPlugin::UpdateConfigSettings()
{
	GetConfigValue<uint64_t>("SleepMS", _sleepMS);
	GetConfigValue<uint32_t>("NTCIPPort", _ntcipPort);
	GetConfigValue<uint64_t>("BSMTimeoutMS", _bsmTimeoutMS);
	GetConfigValue<uint64_t>("SRMTimeoutMS", _srmTimeoutMS);
	GetConfigValue<uint64_t>("PhaseControlGroupCount", _phaseControlGroupCount);
	GetConfigValue<uint64_t>("PhaseCallDurationMS", _phaseCallDurationMS);
	GetConfigValue<uint64_t>("PreemptCount", _preemptCount);
	GetConfigValue<bool>("ClearPreemptsOnStartup", _clearPreemptsOnStartup);
	{
		lock_guard<mutex> lock(_configDataLock);
		GetConfigValue<string>("NTCIPAddress", _ntcipAddress);
		GetConfigValue<string>("SignalGroupPolicy", _signalGroupPolicyJSON);
		GetConfigValue<string>("PreemptionPolicy", _preemptionPolicyJSON);
	}
	_configChanged = true;
}

void MmitssRSPlugin::OnConfigChanged(const char *key, const char *value)
{
	TmxMessageManager::OnConfigChanged(key, value);
	UpdateConfigSettings();
}


void MmitssRSPlugin::OnStateChange(IvpPluginState state)
{
	TmxMessageManager::OnStateChange(state);

	if (state == IvpPluginState_registered)
	{
		UpdateConfigSettings();
	}
}

uint64_t MmitssRSPlugin::GetMsTimeSinceEpoch()
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return (uint64_t) ((double) (tv.tv_sec) * 1000
			+ (double) (tv.tv_usec) / 1000);
}

// This is the default incoming message handler that should assign the message to a thread for
// decoding and further processing
void MmitssRSPlugin::OnMessageReceived(IvpMessage *msg)
{
	PluginClient::OnMessageReceived(msg);

	string j2735Type = msg->subtype;
	PLOG(logDEBUG2) << "Received J2735 " << j2735Type << " message";

	int msgId = factory.GetMessageId(j2735Type);
	if (msgId < 0) msgId = 0;

	IncomingMessage(msg, 0, 0, msg->timestamp);
}

void MmitssRSPlugin::SendApplicationMessage(ApplicationTypes applicationType, EventCodeTypes eventCode, Severity sev, std::string txt, std::string interaction, uint64_t time)
{
	static routeable_message timer;
	timer.refresh_timestamp();

	ApplicationMessage msg;
	msg.set_Id(NewGuid());
	msg.set_AppId(applicationType);
	msg.set_EventCode(eventCode);
	msg.set_Severity(sev);
	msg.set_CustomText(txt);
	if (!interaction.empty())
		msg.set_InteractionId(interaction);
	if (time > 0)
		msg.set_Timestamp(to_string(time));
	else
		msg.set_Timestamp(to_string(timer.get_timestamp()));

	BroadcastMessage(msg);

	PLOG(logDEBUG) << "SendApplicationMessage: " << msg;
}


void MmitssRSPlugin::CheckCall(double lat, double lon, double speedMetersPS, double headingDegrees, uint32_t vehicleId)
{
	uint64_t currentTimeMS = GetMsTimeSinceEpoch();
	int signalGroup = _activeIntersection->GetSignalGroupForLocation(lat, lon);
	if (signalGroup != -1)
	{
		//check if calls allowed on this signal group
		if (_signalGroupPolicy.find(signalGroup) != _signalGroupPolicy.end() && _signalGroupPolicy[signalGroup].callPhase != -1)
		{
			//check if within call frequency
			if (currentTimeMS - _signalGroupPolicy[signalGroup].lastCallTimeMS >= _callMaxFrequencyMS)
			{
				//get signal state
				Signal signalState = _activeIntersection->GetSignalForSignalGroup(signalGroup);
				//check if not green
				if (signalState == Signal::Red || signalState == Signal::Yellow) // include yellow?
				{
					//check if vehicle has called this signal group
					if (_vehicles[vehicleId].signalGroupCalls.find(signalGroup) == _vehicles[vehicleId].signalGroupCalls.end())
					{
						//check time to intersection
						if (_activeIntersection->GetDistanceToIntersection(lat, lon) / speedMetersPS * 1000 <= _callMaxTimeToIntersectionMS)
						{
							//call signal group
							CallPhase(_signalGroupPolicy[signalGroup].callPhase);
							PLOG(logDEBUG) << "Send Call, signalGroup: " << signalGroup << ", vehicleId: " << vehicleId << ", phase: " << _signalGroupPolicy[signalGroup].callPhase;

							//add signal group to vehicles calls and note time
							_vehicles[vehicleId].signalGroupCalls.insert(signalGroup);
							_signalGroupPolicy[signalGroup].lastCallTimeMS = currentTimeMS;

						}
					}
				}
				else if (signalState == Signal::Green)
				{
					//TEST
					//PLOG(logDEBUG) << "Green time remaining: " << _activeIntersection->TimeRemainingForSignalGroup(signalGroup);
				}
			}
		}
	}

}

void MmitssRSPlugin::CheckExtend(double lat, double lon, double speedMetersPS, double headingDegrees, uint32_t vehicleId, bool useMaxExtend)
{
	bool cancelCurrentExtend = false;
	bool activateNewExtend = false;
	uint64_t extendMaxEndTimestampMS = 0;
	uint64_t currentTimeMS = GetMsTimeSinceEpoch();

	//get signal group
	int signalGroup = _activeIntersection->GetSignalGroupForLocation(lat, lon);
	if (signalGroup != -1)
	{
		//check if extend allowed on this group and preempt not active
		if (_signalGroupPolicy.find(signalGroup) != _signalGroupPolicy.end() && _signalGroupPolicy[signalGroup].extendPhase != -1 && _acticvePreempts.empty())
		{
			//get signal state
			Signal signalState = _activeIntersection->GetSignalForSignalGroup(signalGroup);
			//check if green
			if (signalState == Signal::Green)
			{
				int64_t timeToIntersectionMS = _activeIntersection->GetDistanceToIntersection(lat, lon) / speedMetersPS * 1000;
				int64_t timeToIndecisionStart = timeToIntersectionMS - 5500; //5.5 seconds from stop bar
				int64_t timeToIndecisionEnd = timeToIntersectionMS - 2500; //2.5 seconds from stop bar
				int64_t phaseTimeRemainingMS = _activeIntersection->TimeRemainingForSignalGroup(signalGroup) * 1000;
				//PLOG(logDEBUG) << "Extend info, timeToIntersectionMS: " << timeToIntersectionMS << ", phaseTimeRemainingMS: " << phaseTimeRemainingMS << ", maxExtendTimeMS: " << _signalGroupPolicy[signalGroup].maxExtendTimeMS;
				//check if we should extend
				if (timeToIndecisionEnd > 0 &&
						phaseTimeRemainingMS < timeToIndecisionEnd &&
						((!useMaxExtend && phaseTimeRemainingMS > timeToIndecisionStart) ||
								(useMaxExtend && phaseTimeRemainingMS > timeToIndecisionStart)))

				{
					//check if we have an active extend or extend signal groups dont match
					if (!_vehicles[vehicleId].extendActive || _vehicles[vehicleId].extendSignalGroup != signalGroup)
					{
						//activate new extend
						extendMaxEndTimestampMS = currentTimeMS + phaseTimeRemainingMS + _signalGroupPolicy[signalGroup].maxExtendTimeMS;
						activateNewExtend = true;
					}
				}
				else
				{
					//extend not needed, cancel any active extend
					if (_vehicles[vehicleId].extendActive)
					{
						//cancel extend
						cancelCurrentExtend = true;
					}
				}
			}
			else
			{
				//signal not green, cancel any active extend
				if (_vehicles[vehicleId].extendActive)
				{
					//cancel extend
					cancelCurrentExtend = true;
				}
			}
		}
	}

	//check if we need to cancel current extend due to preempt active or we are not in active extend signal group
	if (_vehicles[vehicleId].extendActive)
	{
		if (!_acticvePreempts.empty() || _vehicles[vehicleId].extendSignalGroup != signalGroup)
		{
			//cancel extend
			cancelCurrentExtend = true;
		}
	}

	//cancel current extend
	if (cancelCurrentExtend)
	{
		PLOG(logDEBUG) << "Cancel Extend, phase: "  << _signalGroupPolicy[_vehicles[vehicleId].extendSignalGroup].extendPhase;
		CancelExtend(_signalGroupPolicy[_vehicles[vehicleId].extendSignalGroup].extendPhase);
		_vehicles[vehicleId].extendActive = false;
	}

	//activate new extend
	if (activateNewExtend)
	{
		PLOG(logDEBUG) << "Activate Extend, signalGroup: " << signalGroup << ", vehicleId: " << vehicleId << ", phase: " << _signalGroupPolicy[signalGroup].extendPhase;
		ActivateExtend(_signalGroupPolicy[signalGroup].extendPhase, extendMaxEndTimestampMS);
		_vehicles[vehicleId].extendActive = true;
		_vehicles[vehicleId].extendSignalGroup = signalGroup;
	}

}

// This is the callback for each thread that processes a new message of the given type and subtype
void MmitssRSPlugin::OnMessageReceived(const tmx::routeable_message &rMsg)
{
	routeable_message routeableMsg { rMsg };
	string subtype = routeableMsg.get_subtype();
	double lat;
	double lon;
	double speedMetersPS;
	double headingDegrees;
	uint32_t vehicleId;
	uint32_t role;

	if (IsJ2735Message(routeableMsg))
	{
		//PLOG(logDEBUG) << "Decoding J2735 " << subtype << " message: " << routeableMsg.get_payload_bytes();

		if (!_haveSignalGroupPolicy || !IsPluginState(IvpPluginState_registered))
			return;

		if (subtype == tmx::messages::api::MSGSUBTYPE_BASICSAFETYMESSAGE_STRING)
		{
			//PLOG(logDEBUG) << "Process BSM";

			uint64_t currentTimeMS = GetMsTimeSinceEpoch();

			if (_haveMap && _haveSpat)
			{
				//decode
				BsmEncodedMessage encMsg(routeableMsg.get_message());
				BsmMessage bsmMsg = encMsg.decode_j2735_message();

				vehicleId = *((uint32_t*)(bsmMsg.get_j2735_data()->coreData.id.buf));
				lat = bsmMsg.get_j2735_data()->coreData.lat / 10000000.0;
				lon = bsmMsg.get_j2735_data()->coreData.Long / 10000000.0;
				speedMetersPS = bsmMsg.get_j2735_data()->coreData.speed * 0.02;
				headingDegrees = bsmMsg.get_j2735_data()->coreData.heading * 0.0125;

				//add vehicle to list or update its BSM time
				if (_vehicles.find(vehicleId) == _vehicles.end())
				{
					VehicleDataStruct vehicle;
					vehicle.lastBSMTimeMS = currentTimeMS;
					_vehicles[vehicleId] = vehicle;
				}
				else
				{
					_vehicles[vehicleId].lastBSMTimeMS = currentTimeMS;
				}

				//check if we need to make a call
				CheckCall(lat, lon, speedMetersPS, headingDegrees, vehicleId);

				//check if we need to make an extend
				CheckExtend(lat, lon, speedMetersPS, headingDegrees, vehicleId, false);
			}


		}
		else if (subtype == tmx::messages::api::MSGSUBTYPE_MAPDATA_STRING)
		{
			//PLOG(logDEBUG) << "Process MAP";

			int intersectionId = -1;
			bool newMap = false;

			MapDataEncodedMessage encMsg(routeableMsg.get_message());
			MapDataMessage mapMsg = encMsg.decode_j2735_message();


			if (mapMsg.get_j2735_data() && mapMsg.get_j2735_data()->intersections && mapMsg.get_j2735_data()->intersections->list.count > 0)
				intersectionId = mapMsg.get_j2735_data()->intersections->list.array[0]->id.id;
			//PLOG(logDEBUG) << "got map, intersectionId: " << intersectionId;
			if (intersectionId != -1)
			{
				if (_activeIntersection != NULL)
				{
					//have MAP
					//should we keep updating the map?
	//				newMap = _activeIntersection->LoadMap(mapMsg);
	//				if (_activeIntersection->IsMapLoaded())
	//				{
	//					_haveMap = true;
	//				}
	//				else
	//				{
	//					delete _activeIntersection;
	//					_activeIntersection = NULL;
	//					_haveMap = false;
	//				}
				}
				else
				{
					//new MAP
					_activeIntersection = new Intersection;
					newMap = _activeIntersection->LoadMap(mapMsg);

					if (newMap && _activeIntersection->IsMapLoaded())
					{
						_haveMap = true;
					}
					else
					{
						delete _activeIntersection;
						_activeIntersection = NULL;
						_haveMap = false;
					}
				}
			}

		}
		else if (subtype == tmx::messages::api::MSGSUBTYPE_SIGNALPHASEANDTIMINGMESSAGE_STRING)
		{
			//PLOG(logDEBUG) << "Process SPAT";

			bool spatUpdated = false;

			//process SPAT only if we have a map
			if (_haveMap)
			{
				SpatEncodedMessage encMsg(routeableMsg.get_message());
				SpatMessage spatMsg = encMsg.decode_j2735_message();

				spatUpdated = _activeIntersection->UpdateSpat(spatMsg);
				//only set spat received if spat was for active map
				if (spatUpdated && _activeIntersection->IsSpatLoaded())
				{
					//PLOG(logDEBUG) << "updated spat";
					_lastSpat = GetMsTimeSinceEpoch();
					_haveSpat = true;
				}

			}

		}
		else if (subtype == tmx::messages::api::MSGSUBTYPE_SIGNALREQUESTMESSAGE_STRING)
		{
			//PLOG(logDEBUG) << "Process SRM";

			if (_haveMap && _haveSpat)
			{
				/// action: decode SRM, set processSRM flag (priority request server), updateSrmList
				SrmEncodedMessage encMsg(routeableMsg.get_message());
				SrmMessage srmMsg = encMsg.decode_j2735_message();

				vehicleId = *((uint32_t*)(srmMsg.get_j2735_data()->requestor.id.choice.entityID.buf));
				//check if have all data needed
				if (srmMsg.get_j2735_data()->requestor.position == NULL || srmMsg.get_j2735_data()->requestor.position->speed == NULL ||
						srmMsg.get_j2735_data()->requestor.position->heading == NULL || srmMsg.get_j2735_data()->requestor.type == NULL)
				{
					return;
				}
				lat = srmMsg.get_j2735_data()->requestor.position->position.lat / 10000000.0;
				lon = srmMsg.get_j2735_data()->requestor.position->position.Long / 10000000.0;
				speedMetersPS = srmMsg.get_j2735_data()->requestor.position->speed->speed * 0.02;
				headingDegrees = *(srmMsg.get_j2735_data()->requestor.position->heading) * 0.0125;
				role = srmMsg.get_j2735_data()->requestor.type->role;

				//PLOG(logDEBUG) << "Process SRM: " << vehicleId << ", " << lat << ", " << lon << ", " << speedMetersPS << ", " << headingDegrees << ", " << role;

				uint64_t currentTimeMS = GetMsTimeSinceEpoch();

				//add vehicle to list or update its SRM time
				if (_vehicles.find(vehicleId) == _vehicles.end())
				{
					VehicleDataStruct vehicle;
					vehicle.lastSRMTimeMS = currentTimeMS;
					if (_vehicles[vehicleId].preemptRole == BasicVehicleRole::BasicVehicleRole_ambulance ||
							_vehicles[vehicleId].preemptRole == BasicVehicleRole::BasicVehicleRole_fire ||
							_vehicles[vehicleId].preemptRole == BasicVehicleRole::BasicVehicleRole_police)
						vehicle.preemptRequested = true;
					vehicle.preemptRole = role;
					_vehicles[vehicleId] = vehicle;
				}
				else
				{
					_vehicles[vehicleId].lastSRMTimeMS = currentTimeMS;
					if (_vehicles[vehicleId].preemptRole == BasicVehicleRole::BasicVehicleRole_ambulance ||
							_vehicles[vehicleId].preemptRole == BasicVehicleRole::BasicVehicleRole_fire ||
							_vehicles[vehicleId].preemptRole == BasicVehicleRole::BasicVehicleRole_police)
						_vehicles[vehicleId].preemptRequested = true;
					_vehicles[vehicleId].preemptRole = role;
				}

				//check min speed
				if (speedMetersPS >= _minVehicleSpeedMetersPS)
				{
					//check if preempt allowed for this vehicle type
					if (_vehicles[vehicleId].preemptRole == BasicVehicleRole::BasicVehicleRole_ambulance ||
							_vehicles[vehicleId].preemptRole == BasicVehicleRole::BasicVehicleRole_fire ||
							_vehicles[vehicleId].preemptRole == BasicVehicleRole::BasicVehicleRole_police)
					{
						MapMatchResult mapApproachLane = _activeIntersection->GetApproachLaneForLocation(lat, lon, headingDegrees);

						//PLOG(logDEBUG) << "mapApproachLane LaneNumber: " << mapApproachLane.LaneNumber << ", IsInLane: " << mapApproachLane.IsInLane << ", lat,long: " << lat << "," << lon;

						//check if preempt is not activated
						if (_vehicles[vehicleId].preemptActivated == false)
						{
							//check if we have a policy for this approach lane
							if (mapApproachLane.IsInLane && mapApproachLane.LaneNumber > 0 && _preemptionPolicy.find(mapApproachLane.LaneNumber) != _preemptionPolicy.end())
							{
								int32_t preemptionPolicy = -1;
								if (_vehicles[vehicleId].preemptRole == BasicVehicleRole::BasicVehicleRole_ambulance)
									preemptionPolicy = _preemptionPolicy[mapApproachLane.LaneNumber].preemptionAmbulance;
								else if (_vehicles[vehicleId].preemptRole == BasicVehicleRole::BasicVehicleRole_fire)
									preemptionPolicy = _preemptionPolicy[mapApproachLane.LaneNumber].preemptionFire;
								else if (_vehicles[vehicleId].preemptRole == BasicVehicleRole::BasicVehicleRole_police)
									preemptionPolicy = _preemptionPolicy[mapApproachLane.LaneNumber].preemptionPolice;
								//check if preempt policy available for this vehicle role
								if (preemptionPolicy != -1)
								{
									//activate this preempt
									ActivatePreempt(preemptionPolicy);
									PLOG(logDEBUG) << "Activate preempt, preemptionPolicy: " << preemptionPolicy << ", vehicleId: " << vehicleId << ", mapApproachLane: " << mapApproachLane.LaneNumber;
									_vehicles[vehicleId].preemptActivated = true;
									_vehicles[vehicleId].preemptActivationTimeMS = currentTimeMS;
									_vehicles[vehicleId].preemptPolicy = preemptionPolicy;
									_acticvePreempts[vehicleId] = preemptionPolicy;
								}
							}
						}
						else //preempt is activated
						{
							bool cancelPreempt = false;
							//check if we are in a different preempt policy
							if (mapApproachLane.IsInLane && mapApproachLane.LaneNumber > 0 && _preemptionPolicy.find(mapApproachLane.LaneNumber) != _preemptionPolicy.end())
							{
								int32_t preemptionPolicy = -1;
								if (_vehicles[vehicleId].preemptRole == BasicVehicleRole::BasicVehicleRole_ambulance)
									preemptionPolicy = _preemptionPolicy[mapApproachLane.LaneNumber].preemptionAmbulance;
								else if (_vehicles[vehicleId].preemptRole == BasicVehicleRole::BasicVehicleRole_fire)
									preemptionPolicy = _preemptionPolicy[mapApproachLane.LaneNumber].preemptionFire;
								else if (_vehicles[vehicleId].preemptRole == BasicVehicleRole::BasicVehicleRole_police)
									preemptionPolicy = _preemptionPolicy[mapApproachLane.LaneNumber].preemptionPolice;
								if (preemptionPolicy == -1 || preemptionPolicy != _vehicles[vehicleId].preemptPolicy)
								{
									cancelPreempt = true;
								}
							}
							else
							{
								//no policy for location and heading
								cancelPreempt = true;
							}
							if (cancelPreempt)
							{
								//cancel this preempt
								CancelPreempt(_vehicles[vehicleId].preemptPolicy);
								PLOG(logDEBUG) << "Cancel preempt, preemptionPolicy: " << _vehicles[vehicleId].preemptPolicy << ", vehicleId: " << vehicleId;
								_vehicles[vehicleId].preemptActivated = false;
								_acticvePreempts.erase(vehicleId);
							}

						}

					}
					else if (_vehicles[vehicleId].preemptRole == BasicVehicleRole::BasicVehicleRole_transit)  // transit green extend
					{
						//check if we need to make an extend
						CheckExtend(lat, lon, speedMetersPS, headingDegrees, vehicleId, true);
					}

				}

			}

		}
	}
	else //get a message from main thread
	{
		//PLOG(logDEBUG) << "Received " << routeableMsg.get_type() << " " << routeableMsg.get_subtype() << ": " << routeableMsg.get_payload<tmx::message>();
		stringstream ss;
		ss << routeableMsg.get_payload<tmx::message>();
		string payload = ss.str();
		if (payload.find("CONFIGCHANGED") != string::npos)
		{
			PLOG(logDEBUG) << "Process CONFIGCHANGED";
			string signalGroupPolicyJSON = "";
			string preemptionPolicyJSON = "";
			{
				lock_guard<mutex> lock(_configDataLock);
				if (_ntcipAddress != _ntcipAddressCopy || _ntcipPort != _ntcipPortCopy)
					_snmpDestinationChanged = true;
				_ntcipAddressCopy = _ntcipAddress;
				_ntcipPortCopy = _ntcipPort;
				signalGroupPolicyJSON = _signalGroupPolicyJSON;
				preemptionPolicyJSON = _preemptionPolicyJSON;
			}
			if (signalGroupPolicyJSON != "" && signalGroupPolicyJSON != _signalGroupPolicyJSONCopy)
			{
				_signalGroupPolicy.clear();
				_haveSignalGroupPolicy = false;
				_signalGroupPolicyJSONCopy = signalGroupPolicyJSON;
				PLOG(logDEBUG) << "SignalGroupPolicy: " << _signalGroupPolicyJSONCopy;
				//parse signal group policy
				uint32_t signalGroupId;
				SignalGroupPolicyDataStruct policy;
				ptree pt;
				istringstream is(_signalGroupPolicyJSONCopy);
				read_json(is, pt);
				BOOST_FOREACH(ptree::value_type &signalGroup, pt.get_child("SignalGroups"))
				{
					signalGroupId = signalGroup.second.get<uint32_t>("SignalGroupId");
					PLOG(logDEBUG) << "Process signal group " << signalGroupId;
					policy.callPhase = signalGroup.second.get<int32_t>("CallPhase", -1);
					policy.extendPhase = signalGroup.second.get<int32_t>("ExtendPhase", -1);
					policy.maxExtendTimeMS = signalGroup.second.get<int32_t>("MaxExtendTimeMS", 0);
					policy.lastCallTimeMS = 0;
					_signalGroupPolicy[signalGroupId] = policy;
				}
				if (_signalGroupPolicy.size() > 0)
					_haveSignalGroupPolicy = true;

			}
			if (preemptionPolicyJSON != "" && preemptionPolicyJSON != _preemptionPolicyJSONCopy)
			{
				_preemptionPolicy.clear();
				_havePreemptionPolicy = false;
				_preemptionPolicyJSONCopy = preemptionPolicyJSON;
				PLOG(logDEBUG) << "PreemptionPolicy: " << _preemptionPolicyJSONCopy;
				//parse preemption policy
				uint32_t mapLaneId;
				PreemptionPolicyDataStruct policy;
				ptree pt;
				istringstream is(_preemptionPolicyJSONCopy);
				read_json(is, pt);
				BOOST_FOREACH(ptree::value_type &mapLane, pt.get_child("MapLanes"))
				{
					mapLaneId = mapLane.second.get<uint32_t>("MapLaneId");
					PLOG(logDEBUG) << "Process map laneId " << mapLaneId;
					policy.preemptionAmbulance = mapLane.second.get<int32_t>("PreemptionAmbulance", -1);
					policy.preemptionFire = mapLane.second.get<int32_t>("PreemptionFire", -1);
					policy.preemptionPolice = mapLane.second.get<int32_t>("PreemptionPolice", -1);
					_preemptionPolicy[mapLaneId] = policy;
				}
				if (_preemptionPolicy.size() > 0)
					_havePreemptionPolicy = true;

			}

		}
		else if (payload.find("WATCHDOG") != string::npos)
		{
			//PLOG(logDEBUG) << "Process WATCHDOG";
			uint64_t currentTimeMS = GetMsTimeSinceEpoch();

			//check for preempt SRM timeout
			if (!_acticvePreempts.empty())
			{
				auto acticvePreemptsIt = _acticvePreempts.begin();
				while (acticvePreemptsIt != _acticvePreempts.end())
				{
					if (currentTimeMS - _vehicles[acticvePreemptsIt->first].lastSRMTimeMS >= _srmTimeoutMS)
					{
						//cancel this preempt
						CancelPreempt(_vehicles[acticvePreemptsIt->first].preemptPolicy);
						PLOG(logDEBUG) << "Cancel preempt, preemptionPolicy: " << _vehicles[acticvePreemptsIt->first].preemptPolicy << ", vehicleId: " << acticvePreemptsIt->first;
						_vehicles[acticvePreemptsIt->first].preemptActivated = false;
						if (currentTimeMS - _vehicles[acticvePreemptsIt->first].lastSRMTimeMS >= _srmTimeoutMS)
							_vehicles[acticvePreemptsIt->first].preemptRequested = false;
						acticvePreemptsIt = _acticvePreempts.erase(acticvePreemptsIt);
					}
					else
						++acticvePreemptsIt;
				}
			}

			//check phase call duration timeouts
			if (_callsActive)
			{
				_callsActive = false;
				for (int i = 0;i < _phaseControlGroupCount * 8;i++)
				{
					if (_currentVehicleCalls[i] == true)
					{
						if (currentTimeMS - _currentVehicleLastCallTimesMS[i] >= _phaseCallDurationMS)
						{
							PLOG(logDEBUG) << "Clear Call, phase: "  << i + 1;
							ClearCall(i + 1);
						}
						else
						{
							_callsActive = true;
						}
					}
				}
			}

			//check phase hold duration timeouts
			if (_extendsActive)
			{
				_extendsActive = false;
				for (int i = 0;i < _phaseControlGroupCount * 8;i++)
				{
					if (_currentVehicleExtendCounts[i] > 0)
					{
						//if we still have active extends and have gone past max timestamp cancel hold
						if (currentTimeMS > _currentVehicleExtendMaxEndTimestampMS[i])
						{
							PLOG(logDEBUG) << "Cancel Extend (forced), phase: "  << i + 1;
							CancelExtend(i + 1, true);
							//loop through all vehicles and deactivate extends for this phase
							auto _vehiclesIt = _vehicles.begin();
							while (_vehiclesIt != _vehicles.end())
							{
								if (_vehiclesIt->second.extendActive && _signalGroupPolicy[_vehiclesIt->second.extendSignalGroup].extendPhase == (i + 1))
								{
									_vehiclesIt->second.extendActive = false;
								}
								++_vehiclesIt;
							}
						}
						else
						{
							_extendsActive = true;
						}
					}
				}
			}

			//check vehicle cleanup
			if (currentTimeMS - _lastVehicleCleanupMS > _lastVehicleCleanupIntervalMS)
			{
				auto _vehiclesIt = _vehicles.begin();
				while (_vehiclesIt != _vehicles.end())
				{
					if (currentTimeMS - _vehiclesIt->second.lastBSMTimeMS >= _bsmTimeoutMS && currentTimeMS - _vehiclesIt->second.lastSRMTimeMS >= _srmTimeoutMS)
					{
						_vehiclesIt = _vehicles.erase(_vehiclesIt);
					}
					else
						++_vehiclesIt;
				}
				_lastVehicleCleanupMS = currentTimeMS;
			}
		}

	}

}

void MmitssRSPlugin::CallPhase(int32_t phase)
{
	_currentVehicleCalls[phase - 1] = true;
	_currentVehicleLastCallTimesMS[phase - 1] = GetMsTimeSinceEpoch();
	_callsActive = true;
	int32_t phaseGroup = phase / 8 + 1;
	int32_t shift = (phase % 8) - 1;
	uint8_t phaseBit = ((uint8_t)1) << shift;
	_currentVehicleCallGroups[phaseGroup - 1] = _currentVehicleCallGroups[phaseGroup - 1] | phaseBit;
	stringstream ss;
	ss << "1.3.6.1.4.1.1206.4.2.1.1.5.1.6." << phaseGroup;
	string oidString = ss.str();
	SNMPSet(oidString, _currentVehicleCallGroups[phaseGroup - 1]);
}

void MmitssRSPlugin::ClearCall(int32_t phase)
{
	//PLOG(logDEBUG) << "CallPhase " << phase;
	_currentVehicleCalls[phase - 1] = false;
	int32_t phaseGroup = phase / 8 + 1;
	int32_t shift = (phase % 8) - 1;
	uint8_t phaseBit = ~(((uint8_t)1) << shift);
	_currentVehicleCallGroups[phaseGroup - 1] = _currentVehicleCallGroups[phaseGroup - 1] & phaseBit;
	stringstream ss;
	ss << "1.3.6.1.4.1.1206.4.2.1.1.5.1.6." << phaseGroup;
	string oidString = ss.str();
	SNMPSet(oidString, _currentVehicleCallGroups[phaseGroup - 1]);
}

void MmitssRSPlugin::ActivateExtend(int32_t phase, uint64_t extendMaxEndTimestampMS)
{
	if (_currentVehicleExtendCounts[phase - 1] == 0)
	{
		//if this is the first extend request for this phase set end timestamp and place hold on phase
		_currentVehicleExtendMaxEndTimestampMS[phase - 1] = extendMaxEndTimestampMS;
		int32_t phaseGroup = phase / 8 + 1;
		int32_t shift = (phase % 8) - 1;
		uint8_t phaseBit = ((uint8_t)1) << shift;
		_currentVehicleExtendGroups[phaseGroup - 1] = _currentVehicleExtendGroups[phaseGroup - 1] | phaseBit;
		stringstream ss;
		ss << "1.3.6.1.4.1.1206.4.2.1.1.5.1.4." << phaseGroup;
		string oidString = ss.str();
		SNMPSet(oidString, _currentVehicleExtendGroups[phaseGroup - 1]);

	}
	_currentVehicleExtendCounts[phase - 1]++;
	_extendsActive = true;
}

void MmitssRSPlugin::CancelExtend(int32_t phase, bool force)
{
	if (_currentVehicleExtendCounts[phase - 1] == 1 || force)
	{
		//if this is the last extend or forced cancel for this phase remove hold on phase
		int32_t phaseGroup = phase / 8 + 1;
		int32_t shift = (phase % 8) - 1;
		uint8_t phaseBit = ~(((uint8_t)1) << shift);
		_currentVehicleExtendGroups[phaseGroup - 1] = _currentVehicleExtendGroups[phaseGroup - 1] & phaseBit;
		stringstream ss;
		ss << "1.3.6.1.4.1.1206.4.2.1.1.5.1.4." << phaseGroup;
		string oidString = ss.str();
		SNMPSet(oidString, _currentVehicleExtendGroups[phaseGroup - 1]);

	}
	if (_currentVehicleExtendCounts[phase - 1] > 0)
		_currentVehicleExtendCounts[phase - 1]--;
	if (force)
		_currentVehicleExtendCounts[phase - 1] = 0;
}

void MmitssRSPlugin::CancelAllActiveExtends()
{
	//loop through all vehicles and deactivate all active extends
	auto _vehiclesIt = _vehicles.begin();
	while (_vehiclesIt != _vehicles.end())
	{
		if (_vehiclesIt->second.extendActive)
		{
			PLOG(logDEBUG) << "Cancel Extend, phase: "  << _signalGroupPolicy[_vehiclesIt->second.extendSignalGroup].extendPhase;
			CancelExtend(_signalGroupPolicy[_vehiclesIt->second.extendSignalGroup].extendPhase);
			_vehiclesIt->second.extendActive = false;
		}
		++_vehiclesIt;
	}

}

void MmitssRSPlugin::ActivatePreempt(int32_t policy)
{
	//cancel all active extends first
	CancelAllActiveExtends();
	stringstream ss;
	ss << "1.3.6.1.4.1.1206.4.2.1.6.3.1.2." << policy;
	string oidString = ss.str();
	SNMPSet(oidString, 1);
}

void MmitssRSPlugin::CancelPreempt(int32_t policy)
{
	stringstream ss;
	ss << "1.3.6.1.4.1.1206.4.2.1.6.3.1.2." << policy;
	string oidString = ss.str();
	SNMPSet(oidString, 0);
}

void MmitssRSPlugin::SNMPOpenSession()
{
	//check for valid TSC info
	if (_ntcipAddressCopy == "" || _ntcipPort == 0)
		return;
	//open snmp session
	snmp_sess_init(&_session_info);
	string peername = _ntcipAddressCopy;
	peername.append(":");
	peername.append(to_string(_ntcipPortCopy));
	_session_info.peername = (char*)peername.c_str();
	_session_info.version = SNMP_VERSION_1;
	_session_info.community = (u_char*)"public";
	_session_info.community_len = strlen("public");
	_session = snmp_open(&_session_info);
	if (_session)
		_snmpSessionOpen = true;
}

void MmitssRSPlugin::SNMPCloseSession()
{
	//close session
	snmp_close(_session);
}

bool MmitssRSPlugin::SNMPSet(string targetOid, int32_t value)
{
	return SNMPSet(targetOid, ASN_INTEGER, (const void *)&value, sizeof(value));
}

bool MmitssRSPlugin::SNMPSet(string targetOid, u_char type, const void *value, size_t len)
{
	struct snmp_pdu *pdu;
	struct snmp_pdu *response;
	oid anOID[MAX_OID_LEN];
	size_t anOID_len = MAX_OID_LEN;
	int status;
	bool rc = true;

	//check is snmp session open
	if (!_snmpSessionOpen)
	{
		SNMPOpenSession();
		if (!_snmpSessionOpen)
			return false;
		_snmpDestinationChanged = false;
	}
	//check destination change
	if (_snmpDestinationChanged)
	{
		SNMPCloseSession();
		SNMPOpenSession();
		if (!_snmpSessionOpen)
			return false;
		_snmpDestinationChanged = false;
	}

	pdu = snmp_pdu_create(SNMP_MSG_SET);
	read_objid(targetOid.c_str(), anOID, &anOID_len);
	snmp_pdu_add_variable(pdu, anOID, anOID_len, type, value, len);
	status = snmp_synch_response(_session, pdu, &response);
	if (status != STAT_SUCCESS || response->errstat != SNMP_ERR_NOERROR)
		rc = false;
	if (response)
		snmp_free_pdu(response);

	return rc;
}

bool MmitssRSPlugin::SNMPGet(string targetOid, bool walk)
{
	struct snmp_pdu *pdu;
	struct snmp_pdu *response;
	oid anOID[MAX_OID_LEN];
	size_t anOID_len = MAX_OID_LEN;
	int status;
	bool rc = false;
    struct variable_list *vars;
    bool success = true;
    bool keepWalking = true;
    uint32_t numProcessed = 0;
    string nextOid = "";
	string targetOidString = "";

	//check is snmp session open
	if (!_snmpSessionOpen)
	{
		SNMPOpenSession();
		if (!_snmpSessionOpen)
			return false;
		_snmpDestinationChanged = false;
	}
	//check destination change
	if (_snmpDestinationChanged)
	{
		SNMPCloseSession();
		SNMPOpenSession();
		if (!_snmpSessionOpen)
			return false;
		_snmpDestinationChanged = false;
	}

	while(success && keepWalking)
	{
		anOID_len = MAX_OID_LEN;
		if (numProcessed == 0)
		{
			//get the first one
			pdu = snmp_pdu_create(SNMP_MSG_GET);
			int parsed = read_objid(targetOid.c_str(), anOID, &anOID_len);
			if (parsed != 1)
			{
				PLOG(logDEBUG) << "SNMPGET " << targetOid << ", parse failed";
				success = false;
			}
			else
			{
				//save the oid string as printed by the library
				char oidString[1024];
				int len = snprint_objid(oidString, 1024, anOID, anOID_len);
				if (len > 0)
				{
					targetOidString = string(oidString, len);
				}
				else
				{
					PLOG(logDEBUG) << "SNMPGET " << targetOid << ", snprint_objid failed";
					success = false;
				}
			}
		}
		else
		{
			pdu = snmp_pdu_create(SNMP_MSG_GETNEXT);
			int parsed = read_objid(nextOid.c_str(), anOID, &anOID_len);
			if (parsed != 1)
			{
				PLOG(logDEBUG) << "SNMPGETNEXT " << nextOid << ", parse failed";
				success = false;
			}
		}
		if (success)
		{
			snmp_add_null_var(pdu, anOID, anOID_len);
			status = snmp_synch_response(_session, pdu, &response);
			if (status == STAT_SUCCESS && response->errstat == SNMP_ERR_NOERROR)
			{
				nextOid = "";
				for(vars = response->variables; vars; vars = vars->next_variable)
				{
					char oidString[1024];
					int len = snprint_objid(oidString, 1024, vars->name, vars->name_length);
					if (len > 0)
					{
						nextOid = string(oidString, len);
						//check if this oid is in the same tree
						if (nextOid.length() < targetOidString.length() || nextOid.substr(0, targetOidString.length()) != targetOidString)
						{
							keepWalking = false;
						}
						else
						{
							string msg = nextOid;
							if (vars->type == ASN_OCTET_STR)
							{
								string s((const char*)vars->val.string, vars->val_len);
								stringstream ss;
								ss << " STRING(" << vars->val_len << ") : " << s;
								msg.append(ss.str());
							}
							else if (vars->type == ASN_BOOLEAN)
							{
								stringstream ss;
								ss << " BOOL(" << vars->val_len << ") : ";
								msg.append(ss.str());
							}
							else if (vars->type == ASN_INTEGER)
							{
								stringstream ss;
								ss << " INT(" << vars->val_len << ") : " << *vars->val.integer;
								msg.append(ss.str());

							}
							else
							{
								stringstream ss;
								ss << " " << vars->type << "(" << vars->val_len << ") : ";
								msg.append(ss.str());
							}
							PLOG(logDEBUG) << msg;
						}
					}
					else
					{
						success = false;
					}
				}
				rc = true;
			}
			else if (numProcessed == 0)
			{
				//first get failed, set oid for walk
				nextOid = targetOidString;
			}
			else
			{
				success = false;
			}

			if (response)
				snmp_free_pdu(response);

		}

		//check if we want to continue walking
		if (!walk)
			keepWalking = false;

		numProcessed++;
	}

	return rc;
}



int MmitssRSPlugin::Main()
{
	PLOG(logINFO) << "Starting plugin.";


	while (_plugin->state != IvpPluginState_error)
	{
//		if (_haveMap)
//			PLOG(logDEBUG) << "Have MAP";
//		if (_haveSpat)
//			PLOG(logDEBUG) << "Have SPAT";
		//check for config changed
		if (IsPluginState(IvpPluginState_registered))
		{
			//perform initializations
			if (!_initialized)
			{
				_currentVehicleCallGroups = new uint8_t[_phaseControlGroupCount];
				_currentVehicleCalls = new bool[_phaseControlGroupCount * 8];
				_currentVehicleLastCallTimesMS = new uint64_t[_phaseControlGroupCount * 8];
				_currentVehicleExtendGroups = new uint8_t[_phaseControlGroupCount];
				_currentVehicleExtendCounts = new int32_t[_phaseControlGroupCount * 8];
				_currentVehicleExtendMaxEndTimestampMS = new uint64_t[_phaseControlGroupCount * 8];
				//clear all ntcip 1202 vehicle calls
				for (int i = 1;i <= _phaseControlGroupCount;i++)
				{
					_currentVehicleCallGroups[i - 1] = 0;
					stringstream ss;
					ss << "1.3.6.1.4.1.1206.4.2.1.1.5.1.6." << i;
					string oidString = ss.str();
					SNMPSet(oidString, 0);
				}
				for (int i = 0;i < _phaseControlGroupCount * 8;i++)
				{
					_currentVehicleCalls[i] = false;
					_currentVehicleLastCallTimesMS[i] = 0;
				}
				//clear all ntcip 1202 phase holds
				for (int i = 1;i <= _phaseControlGroupCount;i++)
				{
					_currentVehicleExtendGroups[i - 1] = 0;
					stringstream ss;
					ss << "1.3.6.1.4.1.1206.4.2.1.1.5.1.4." << i;
					string oidString = ss.str();
					SNMPSet(oidString, 0);
				}
				for (int i = 0;i < _phaseControlGroupCount * 8;i++)
				{
					_currentVehicleExtendCounts[i] = 0;
					_currentVehicleExtendMaxEndTimestampMS[i] = 0;
				}
				//clear all ntcip 1202 preempts if required, assuming sequential MIB rows
				if (_clearPreemptsOnStartup)
				{
					for (int i = 1;i <= _preemptCount;i++)
						CancelPreempt(i);
				}
				_initialized = true;
			}
			//check for config changed
			if (_configChanged)
			{
				stringstream ss("{\"MESSAGE\":\"CONFIGCHANGED\"}");
				tmx::byte_stream bytes;
				byte_t b;
				while (ss >> b) bytes.push_back(b);
				IncomingMessage(bytes, tmx::messages::api::ENCODING_JSON_STRING);
				_configChanged = false;
			}
			//send watchdog
			stringstream ss("{\"MESSAGE\":\"WATCHDOG\"}");
			tmx::byte_stream bytes;
			byte_t b;
			while (ss >> b) bytes.push_back(b);
			IncomingMessage(bytes, tmx::messages::api::ENCODING_JSON_STRING);

		}

		this_thread::sleep_for(chrono::milliseconds(_sleepMS));
	}

	return (EXIT_SUCCESS);
}


} /* namespace MmitssRSPlugin */

int main(int argc, char *argv[])
{
	return run_plugin<MmitssRSPlugin::MmitssRSPlugin>("MmitssRSPlugin", argc, argv);
}
