import logging
import threading
import os
import pyodbc

from azure.servicebus import ServiceBusClient, ServiceBusMessage
from pygnssutils import GNSSNTRIPClient
from time import sleep

class TMXNTRIPClient(GNSSNTRIPClient):
    def __init__(self, hri, xingid, state, lat, lon, **kwargs):
        super().__init__(kwargs)
        self.hri = hri
        self.xingid = xingid
        self.state = state
        self.lat = lat
        self.lon = lon

        self.client = ServiceBusClient.from_connection_string(os.environ["rcvwcbssb_CBSReceiver_SERVICEBUS"])
        self.sender = self.client.get_topic_sender("V2X.RTCM3")

    def _do_write(self, output: object, raw: bytes, parsed: object):
        logging.info("Writing RTCM bytes to crossing " + self.xingid + ": " + raw.hex())
        msg = ServiceBusMessage(body = raw.hex(),
                                content_type = 'RTCM',
                                encoding = 'json',
                                subject = str(self.hri))
        self.sender.send_messages(msg)

    def start(self):
        logging.info("Starting thread for crossing " + self.xingid)
        streaming = super().run(
            server = os.environ['NTRIP_HOST_' + self.state],
            port = os.environ['NTRIP_PORT_' + self.state],
            mountpoint = os.environ['NTRIP_MOUNTPOINT_' + self.state],
            ntripuser = os.environ['NTRIP_USER_' + self.state],
            ntrippassword = os.environ['NTRIP_PASSWORD_' + self.state],
            ggamode = 1,
            ggainterval = 30,
            reflat = self.lat,
            reflon = self.lon
        )
        while streaming:
            sleep(3)

        self.sender.close()
        self.client.close()

logging.basicConfig(format='%(levelname)s:%(message)s', level=logging.DEBUG)

active = { }

def get_conn():
    return pyodbc.connect(os.environ["AZURE_SQL_CONNECTIONSTRING"], autocommit = True)

def run_ntrip(hri):
    logging.info("Running thread for crossing " + active[hri].xingid)
    active[hri].start()

while True:
    logging.info("In loop")
    conn = get_conn()
    cursor = conn.cursor()
    cursor.execute("SELECT HRI_ID, CrossingID, StateName, FixLatitude, FixLongitude "
                   "FROM [dbo].[HRI_CROSSINGS] "
                   "WHERE HRI_ID IS NOT NULL AND RTCMProxy <> 0")
    for row in cursor.fetchall():
        hri = row[0]
        xingid = row[1]
        state = row[2]
        lat = row[3]
        lon = row[4]

        logging.info("Checking " + str(row))
        if 'NTRIP_HOST_' + state not in os.environ:
            continue

        if not hri in active:
            logging.info("Setting up RTCM connection for " + str(hri) + " in " + state + ": Crossing " + xingid)
            active[hri] = TMXNTRIPClient(hri, xingid, state, lat, lon)

            thread = threading.Thread(target = run_ntrip, args = { hri, })
            thread.start()

    cursor.close()
    conn.close()

    # Repeat every 15 minutes to see if new crossings exist
    sleep(15 * 60)
