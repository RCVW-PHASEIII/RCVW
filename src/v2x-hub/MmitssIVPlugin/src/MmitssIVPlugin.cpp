//============================================================================
// Name        : MmitssIVPlugin.cpp
// Author      : Battelle Memorial Institute - Ben Paselsky (paselsky@battelle.org)
// Version     :
// Copyright   : Copyright (c) 2014 Battelle Memorial Institute. All rights reserved.
// Description : Plugin that listens for Location messages and generates
//				 SRM messages
//============================================================================

#include <atomic>
#include <thread>
#include <queue>
#include <curl/curl.h>
#include <boost/regex.hpp>
#include <boost/lockfree/spsc_queue.hpp>

#include "PluginClient.h"
#include <LocationMessage.h>
#include <tmx/j2735_messages/SignalRequestMessage.hpp>
#include <tmx/messages/message_document.hpp>
#include <ApplicationMessageEnumTypes.h>
#include <ApplicationMessage.h>

using namespace std;
using namespace tmx;
using namespace tmx::utils;
using namespace tmx::messages;
using namespace tmx::messages::appmessage;
using namespace std;

namespace MmitssIVPlugin
{

/**
 * This plugin listens for Location messages and generates
 * SRM messages
 */
class MmitssIVPlugin: public PluginClient
{
public:
	MmitssIVPlugin(std::string);
	virtual ~MmitssIVPlugin();
	int Main();
protected:
	void UpdateConfigSettings();
	// Virtual method overrides.
	void OnConfigChanged(const char *key, const char *value);
	void OnStateChange(IvpPluginState state);
	uint64_t GetMsTimeSinceEpoch();
	void HandleLocationMessage(LocationMessage &msg, routeable_message &routeableMsg);
	void SendApplicationMessage(ApplicationTypes applicationType, EventCodeTypes eventCode, Severity sev, std::string txt = "", std::string interaction = "", uint64_t time = 0);

private:
	// locks
	mutex _locationLock;

	atomic<bool> _newConfigValues{false};
	atomic<uint64_t> _locationMsgExpireMs{5000};
	std::atomic<uint64_t> _sleepMS{10}; // sleep in milliseconds
	std::atomic<bool> _enableSRM{100};
	std::atomic<uint32_t> _vehicleID{999111};
	std::atomic<uint32_t> _vehicleRole{14}; // 12 = police, 13 = fire, 14 = ambulance
	std::atomic<uint64_t> _SRMFrequencyMS{100};

	uint64_t _lastSRMSendTimeMS{0};

	LocationMessage _locationMsg;
	uint64_t _locationMsgTime{0};

	uint64_t _locationTimeoutMS{3000};


};

/**
 * Construct a new MmitssIVPlugin with the given name.
 *
 * @param name The name to give the plugin for identification purposes
 */
MmitssIVPlugin::MmitssIVPlugin(string name) : PluginClient(name)
{

	// Add a message filter and handler for each message this plugin wants to receive.
	AddMessageFilter<LocationMessage>(this, &MmitssIVPlugin::HandleLocationMessage);

	// Subscribe to all messages specified by the filters above.
	SubscribeToMessages();

}

MmitssIVPlugin::~MmitssIVPlugin()
{
}

void MmitssIVPlugin::UpdateConfigSettings()
{
	GetConfigValue("LocationMsgExpireMs", _locationMsgExpireMs);
	GetConfigValue<uint64_t>("SleepMS", _sleepMS);
	GetConfigValue<bool>("EnableSRM", _enableSRM);
	GetConfigValue<uint32_t>("VehicleID", _vehicleID);
	GetConfigValue<uint32_t>("VehicleRole", _vehicleRole);
	GetConfigValue<uint64_t>("SRMFrequencyMS", _SRMFrequencyMS);

	//PLOG(logDEBUG) << "    Config data - LocationMsgExpiration: " << _locationMsgExpireMs ;

	_newConfigValues = true;
}

void MmitssIVPlugin::OnConfigChanged(const char *key, const char *value)
{
	PluginClient::OnConfigChanged(key, value);
	UpdateConfigSettings();
}


void MmitssIVPlugin::OnStateChange(IvpPluginState state)
{
	PluginClient::OnStateChange(state);

	if (state == IvpPluginState_registered)
	{
		UpdateConfigSettings();
	}
}

uint64_t MmitssIVPlugin::GetMsTimeSinceEpoch()
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return (uint64_t) ((double) (tv.tv_sec) * 1000
			+ (double) (tv.tv_usec) / 1000);
}

int MmitssIVPlugin::Main()
{
	PLOG(logINFO) << "Starting plugin.";
	//TravelerInformation *tim;
	int i, j, k;
	uint64_t locationTime;
	LocationMessage locationCopy;

	_lastSRMSendTimeMS = GetMsTimeSinceEpoch() - _SRMFrequencyMS;
	while (_plugin->state != IvpPluginState_error)
	{
		if (IsPluginState(IvpPluginState_registered))
		{
			{
				lock_guard<mutex> lock(_locationLock);
				locationTime = _locationMsgTime;
				locationCopy = _locationMsg;
			}

			//check for old expired message
			if (_enableSRM && (GetMsTimeSinceEpoch() - locationTime) < _locationTimeoutMS)
			{
				//check when to send next SRM
				int64_t nextSRMWaitTimeMS = _lastSRMSendTimeMS + _SRMFrequencyMS - GetMsTimeSinceEpoch();
				if (nextSRMWaitTimeMS >= 0)
					this_thread::sleep_for(chrono::milliseconds(nextSRMWaitTimeMS));
				_lastSRMSendTimeMS = GetMsTimeSinceEpoch();

				//send SRM
				uint32_t vehicleId = _vehicleID;
				uint32_t heading = locationCopy.get_Heading() / 0.0125;
				uint32_t speed = locationCopy.get_Speed_kph() / 3.6 * 50;
				int64_t latitude = locationCopy.get_Latitude() * 10000000;
				int64_t longitude = locationCopy.get_Longitude() * 10000000;
				uint32_t role = _vehicleRole;
				SrmEncodedMessage encodedSrm;
				SignalRequestMessage *srm = (SignalRequestMessage *)calloc(1, sizeof(SignalRequestMessage));
				size_t s = sizeof(vehicleId);
				srm->requestor.id.present = VehicleID_PR_entityID;
				srm->requestor.id.choice.entityID.size = s;
				srm->requestor.id.choice.entityID.buf = (uint8_t *)calloc(s, sizeof(uint8_t));
				if (srm->requestor.id.choice.entityID.buf)
					memcpy(srm->requestor.id.choice.entityID.buf, &vehicleId, s);

				srm->requestor.type =
						(struct RequestorType *)calloc(1, sizeof(struct RequestorType));
				if (srm->requestor.type)
				{
					srm->requestor.type->role = (BasicVehicleRole_t)role;
					srm->requestor.position =
						(struct RequestorPositionVector *)calloc(1, sizeof(struct RequestorPositionVector));
					if (srm->requestor.position)
					{
						srm->requestor.position->position.lat = (Latitude_t)latitude;
						srm->requestor.position->position.Long = (Longitude_t)longitude;
						srm->requestor.position->heading =
								(DSRC_Angle_t *)calloc(1, sizeof(DSRC_Angle_t));
						if (srm->requestor.position->heading)
							*(srm->requestor.position->heading) = (DSRC_Angle_t)heading;
						srm->requestor.position->speed =
								(TransmissionAndSpeed *)calloc(1, sizeof(TransmissionAndSpeed));
						if (srm->requestor.position->speed)
						{
							srm->requestor.position->speed->transmisson = TransmissionState_unavailable;
							srm->requestor.position->speed->speed = (Velocity_t)speed;
						}
					}
				}

				SrmMessage srmMsg(srm);
				//FILE_LOG(logDEBUG) << "Sending SRM: " << srmMsg;

				encodedSrm.clear();
				encodedSrm.initialize(srmMsg);
				encodedSrm.set_flags(IvpMsgFlags_RouteDSRC);
				encodedSrm.addDsrcMetadata(172, 0x20);
				routeable_message *sendMsg = &encodedSrm;
				this->BroadcastMessage(*sendMsg);

			}
		}

		this_thread::sleep_for(chrono::milliseconds(_sleepMS));
	}

	return (EXIT_SUCCESS);
}

void MmitssIVPlugin::SendApplicationMessage(ApplicationTypes applicationType, EventCodeTypes eventCode, Severity sev, std::string txt, std::string interaction, uint64_t time)
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

void MmitssIVPlugin::HandleLocationMessage(LocationMessage &msg, routeable_message &routeableMsg)
{
	// Must be in the registered state in order to know the parameter values
	if (_plugin->state != IvpPluginState_registered)
		return;

	if (msg.get_Latitude() == 0 && msg.get_Longitude() == 0)
		return;

	//Save to member variable for later use.
	{
		lock_guard<mutex> lock(_locationLock);
		_locationMsgTime = routeableMsg.get_timestamp();
		_locationMsg = msg;
	}
	//PLOG(logDEBUG) << "LocationMessage: " << routeableMsg;
}


} /* namespace MmitssIVPlugin */

int main(int argc, char *argv[])
{
	return run_plugin<MmitssIVPlugin::MmitssIVPlugin>("MmitssIVPlugin", argc, argv);
}
