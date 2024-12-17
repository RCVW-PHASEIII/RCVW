import logging
import threading
import os
import requests

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

    def _do_output(self, output: object, raw: bytes, parsed: object):
        logging.debug("Writing RTCM bytes to crossing " + self.xingid + ": " + raw.hex())
        msg = ServiceBusMessage(body = raw.hex(),
                                content_type = 'RTCM',
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

if not 'RCVW_HRI' in os.environ:
    logging.error("No HRI identifer specified. Terminating.")
    exit(1)

if not 'RCVW_CBS_API' in os.environ:
    logging.error("No URL specified to the CBS API. Terminating.")
    exit(1)

logging.info("Starting up RTCM proxy for HRI " + str(os.environ['RCVW_HRI']) + "...")

url = 'https://' + str(os.environ['RCVW_CBS_API']) + '/api/Crossings?$filter=HRI_ID%20eq%20'
url = url + str(os.environ['RCVW_HRI']) + '&uuid=6B48C0ED-F13D-4948-999F-372C5E539EF9'

resp = requests.get(url=url)
data = resp.json()

if 'value' in data and len(data['value']) > 0 and 'HRI_ID' in data['value'][0]:
    if 'NTRIP_HOST_' + data['value'][0]['StateName'] not in os.environ:
        logging.error("Unable to connect to NTRIP in state " + data['value'][0]['StateName'])
        exit(1)

    logging.info("Setting up RTCM connection on behalf of Crossing " + data['value'][0]['CrossingID'] +
                 ": " + data['value'][0]['Street'] + " " + data['value'][0]['CityName'] +
                 ", " + data['value'][0]['StateName'])

    proxy = TMXNTRIPClient(data['value'][0]['HRI_ID'],
                           data['value'][0]['CrossingID'],
                           data['value'][0]['StateName'],
                           data['value'][0]['FixLatitude'],
                           data['value'][0]['FixLongitude'])
    proxy.start()
