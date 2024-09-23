import azure.functions as func
import datetime
import logging
import os
import pyodbc
import subprocess
import uuid
import xmltodict

app = func.FunctionApp()

def get_conn():
    return pyodbc.connect(os.environ["AZURE_SQL_CONNECTIONSTRING"], autocommit = True)

def write_message(azservicebus: func.ServiceBusMessage):
    # Write to the table
    conn = get_conn()
    try:
        conn.cursor().execute("INSERT INTO RBS_INCOMING_MESSAGE " +
                              "([Message_ID], [Topic], [Subject], [Content_Type], [Content], [Received], [Stored]) " +
                              "VALUES (CONVERT(uniqueidentifier, ?), ?, ?, ?, ?, " +
                              "CONVERT(datetime, ?, 127) AT TIME ZONE 'UTC', " +
                              "CURRENT_TIMESTAMP AT TIME ZONE 'UTC')",
                              str(uuid.UUID(hex=azservicebus.message_id)),
                            azservicebus.to, azservicebus.label,
                            azservicebus.content_type, azservicebus.get_body().decode('utf-8'),
                            azservicebus.enqueued_time_utc)
    except Exception as e:
        logging.error(e, exc_info = True)

    conn.commit()
    return conn

def decode_message(azservicebus: func.ServiceBusMessage):
    # Decode the message
    result = subprocess.run(["read_j2735", azservicebus.get_body().decode('utf-8')], capture_output=True)
    return xmltodict.parse(result.stdout)


@app.service_bus_topic_trigger(arg_name="azservicebus",
                               subscription_name="rsu-incoming",
                               topic_name="j2735.map",
                               connection="rcvwcbssb_CBSReceiver_SERVICEBUS") 
def j2735_map(azservicebus: func.ServiceBusMessage):
    conn = write_message(azservicebus)
    data = decode_message(azservicebus)

    intxn = data["MessageFrame"]["value"]["MapData"]["intersections"]["IntersectionGeometry"]

    laneID = 0
    signalGroup = 0

    # Look for either a straight through mulit-directional lane or one that is explicitly labeled
    # as the tracked lane
    for lane in intxn['laneSet']['GenericLane']:
        if 'connectsTo' in lane:
            if (lane['laneAttributes']['directionalUse'] == '11' and
                    lane['connectsTo']['Connection']['connectingLane']['maneuver'] == '100000000000') :
                laneID = lane['laneID']
                signalGroup = lane['connectsTo']['Connection']['signalGroup']
            elif (lane['laneAttributes']['sharedWith'] == '0000000010'):
                laneID = lane['laneID']
                signalGroup = lane['connectsTo']['Connection']['signalGroup']


    # Write the MAP information
    conn.cursor().execute("INSERT INTO RBS_INCOMING_MAP " +
                          "([Message_ID], [HRI_ID], [TrackedLane], [SignalGroup], " +
                          "[RefPointLatitude], [RefPointLongitude]) " +
                          "VALUES (CONVERT(uniqueidentifier, ?), ?, ?, ?, ?, ?)",
                          str(uuid.UUID(hex=azservicebus.message_id)), intxn['id']['id'],
                          laneID, signalGroup, int(intxn['refPoint']['lat']) / 10000000,
                          int(intxn['refPoint']['long']) / 10000000)
    conn.commit()
    conn.close()

@app.service_bus_topic_trigger(arg_name="azservicebus",
                               subscription_name="rsu-incoming",
                               topic_name="j2735.spat",
                               connection="rcvwcbssb_CBSReceiver_SERVICEBUS") 
def j2735_spat(azservicebus: func.ServiceBusMessage):
    conn = write_message(azservicebus)
    data = decode_message(azservicebus)

    intxn = data["MessageFrame"]["value"]["SPAT"]["intersections"]["IntersectionState"]
    time = (datetime.datetime(datetime.date.today().year, 1, 1) +
            datetime.timedelta(minutes=int(intxn['moy'])) +
            datetime.timedelta(milliseconds=int(intxn['timeStamp'])))

    signalGroup = 0
    for movement in intxn['states']['MovementState']:
        # From the HRI Status plugin, the tracked lane movement is considered protected
        if 'protected-Movement-Allowed' in movement['state-time-speed']['MovementEvent']['eventState']:
            signalGroup = movement['signalGroup']
        # But the approach lane movement is fully permissive
        if 'permissive-Movement-Allowed' in movement['state-time-speed']['MovementEvent']['eventState']:
            signalGroup = movement['signalGroup']

    # Write the SPAT
    conn.cursor().execute("INSERT INTO RBS_INCOMING_SPAT " +
                          "([Message_ID], [HRI_ID], [IntersectionName], [MsgTime], [ActiveSignalGroup]) " +
                          "VALUES (CONVERT(uniqueidentifier, ?), ?, ?, "
                          "CONVERT(datetime, ?, 126) AT TIME ZONE 'UTC', ?)",
                          str(uuid.UUID(hex=azservicebus.message_id)), intxn['id']['id'], intxn['name'],
                          str(time.isoformat(timespec='milliseconds')), signalGroup)
    conn.commit()
    conn.close()

@app.service_bus_topic_trigger(arg_name="azservicebus",
                               subscription_name="rsu-incoming",
                               topic_name="tmx.plugin.v2x.hristatusplugin.hristatusplugin.error",
                               connection="rcvwcbssb_CBSReceiver_SERVICEBUS")
def hristatus_error(azservicebus: func.ServiceBusMessage):
    write_message(azservicebus)

@app.service_bus_topic_trigger(arg_name="azservicebus",
                               subscription_name="rsu-incoming",
                               topic_name="tmx.plugin.v2x.hristatusplugin.hristatusplugin.status",
                               connection="rcvwcbssb_CBSReceiver_SERVICEBUS")
def hristatus_status(azservicebus: func.ServiceBusMessage):
    write_message(azservicebus)

@app.service_bus_topic_trigger(arg_name="azservicebus",
                               subscription_name="rsu-incoming",
                               topic_name="tmx.plugin.v2x.map.mapplugin.error",
                               connection="rcvwcbssb_CBSReceiver_SERVICEBUS")
def map_error(azservicebus: func.ServiceBusMessage):
    write_message(azservicebus)

@app.service_bus_topic_trigger(arg_name="azservicebus",
                               subscription_name="rsu-incoming",
                               topic_name="tmx.plugin.v2x.map.mapplugin.status",
                               connection="rcvwcbssb_CBSReceiver_SERVICEBUS")
def map_status(azservicebus: func.ServiceBusMessage):
    write_message(azservicebus).close()

@app.service_bus_topic_trigger(arg_name="azservicebus",
                               subscription_name="rsu-incoming",
                               topic_name="tmx.plugin.v2x.rsu.rsuimmediateforwardplugin.error",
                               connection="rcvwcbssb_CBSReceiver_SERVICEBUS")
def rsuimmediateforward_error(azservicebus: func.ServiceBusMessage):
    write_message(azservicebus).close()

@app.service_bus_topic_trigger(arg_name="azservicebus",
                               subscription_name="rsu-incoming",
                               topic_name="tmx.plugin.v2x.rsu.rsuimmediateforwardplugin.status",
                               connection="rcvwcbssb_CBSReceiver_SERVICEBUS")
def rsuimmediateforward_status(azservicebus: func.ServiceBusMessage):
    write_message(azservicebus).close()
