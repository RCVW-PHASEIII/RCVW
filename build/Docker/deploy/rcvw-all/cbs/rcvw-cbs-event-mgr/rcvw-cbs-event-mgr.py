import datetime
import json
import logging
import os
import pyodbc
import threading
import time
from azure.servicebus import ServiceBusClient, ServiceBusMessage

def get_conn():
    return pyodbc.connect(os.environ["AZURE_SQL_CONNECTIONSTRING"], autocommit = True)

def get_sender(topic: str):
    return ServiceBusClient.from_connection_string(os.environ["rcvwcbssb_CBSReceiver_SERVICEBUS"]).get_topic_sender(topic)

def send_message(topic: str, hri: int, subject: str, content: dict):
    props = {
        'timestamp': str(int(time.time() * 1000000))
    }
    msg = ServiceBusMessage(body = json.dumps(content),
                            content_type = 'Properties<Any>',
                            subject = subject,
                            application_properties = props,
                            encoding = 'json')

    if 'code' in content and int(content['code']) != 0:
        logging.error(content)
    else:
        logging.info(content)

    sender = get_sender(topic)
    sender.send_messages(msg)
    sender.close()

def send_event(hri: int, content: dict):
    send_message("cbs.events", hri, str(hri), content)

def execute_sql(dbcursor, sql, *args):
    run_sql = sql.replace('?', '{!r}' ).format(*args)
    try:
        start = time.time()
        dbcursor.execute(sql, *args)
        finish = time.time()
        logging.info(run_sql + ": Took " + str(finish-start) + " seconds")
    except Exception as e:
        logging.error(run_sql + ": Failed with " + str(e))

def monitor_hri(hri: int):
    conn = get_conn()

    while True:
        rbs_check(conn, hri)
        hri_check(conn, hri)

def hri_check(conn: object, hri: int):
    logging.info("Checking pre-emption status on HRI " + str(hri))
    try:
        # First, try to determine if HRI has been activated
        cursor = conn.cursor()
        execute_sql(cursor,
                    "UPDATE HRI "
                    "SET PreemptionStatus = PreemptionStatus ^ 1, "
                    "LastUpdated = CURRENT_TIMESTAMP "
                    "FROM [dbo].[HRI_ACTIVATION_STATUS] HRI "
                    "JOIN [dbo].[RBS_INCOMING_SPAT_STATUS] STAT "
                    "ON HRI.HRI_ID = STAT.HRI_ID "
                    "AND HRI.RBSOperational = 1 "
                    "AND HRI.ErrorCode IS NULL "
                    "AND STAT.ActiveSignalGroup > 0 "
                    "AND STAT.HRIActive = 1 "
                    "WHERE HRI.HRI_ID = ? "
                    "AND HRI.PreemptionStatus <> STAT.HRIActive",
                    hri)

        if cursor.rowcount > 0:
            send_event(hri, {
                'HRI': hri,
                'active': True,
                'code': 1,
                'message': 'Preemption signal activated'
            })

            cursor.close()
            return

        execute_sql(cursor,"UPDATE HRI "
                           "SET PreemptionStatus = PreemptionStatus ^ 1, "
                           "LastUpdated = CURRENT_TIMESTAMP "
                           "FROM [dbo].[HRI_ACTIVATION_STATUS] HRI "
                           "JOIN [dbo].[RBS_INCOMING_SPAT_STATUS] STAT "
                           "ON HRI.HRI_ID = STAT.HRI_ID "
                           "AND HRI.RBSOperational = 1 "
                           "AND HRI.ErrorCode IS NULL "
                           "AND STAT.ActiveSignalGroup > 0 "
                           "AND STAT.HRIActive = 0 "
                           "WHERE HRI.HRI_ID = ? "
                           "AND HRI.PreemptionStatus <> STAT.HRIActive",
                    hri)

        if cursor.rowcount > 0:
            send_event(hri, {
                'HRI': hri,
                'active': False,
                'code': 0,
                'message': 'Preemption signal deactivated'
            })

        cursor.close()
    except Exception as ex:
        logging.error(str(ex))

def check_rate_failed(conn: object, hri: int, code: int, chk: str, topic: str):
    try:
        if chk == "MAP" or chk == "SPaT":
            err = "J2735 " + chk
            table = '[dbo].[RBS_INCOMING_' + chk.upper() + '_RATE]'
            rate = os.environ['MINIMUM_' + chk.upper() + '_RATE']
            topic = ""
        else:
            table = '[dbo].[RBS_INCOMING_MESSAGE_RATE]'
            rate = os.environ['MINIMUM_MSG_RATE']
            topic = "AND RATE.Topic = '" + topic + "'"

        err = ('RBS ' + chk +
               ' messaging rate has dropped below the minimal operational threshold of ' +
               rate + ' messages per second');

        # First, check if the operational state thresholds are still above minimums
        cursor = conn.cursor()
        execute_sql(cursor, "UPDATE HRI "
                            "SET RBSOperational = RBSOperational ^ 1, "
                            "ErrorCode = ?, "
                            "ErrorMessage = ?, "
                            "LastUpdated = CURRENT_TIMESTAMP "
                            "FROM [dbo].[HRI_ACTIVATION_STATUS] HRI "
                            "JOIN " + table + " RATE "
                                              "ON HRI.HRI_ID = RATE.HRI_ID "
                                              "AND HRI.ErrorCode IS NULL "
                                              "AND HRI.RBSOperational = 1 "
                                              "AND RATE.MsgRate < ? "
                                              "WHERE HRI.HRI_ID = ? " + topic,
                    code + 1, err, rate, hri)

        if cursor.rowcount > 0:
            send_event(hri, {
                'HRI': hri,
                'code': code + 1,
                'message': err
            })

            cursor.close()
            return

        # Next, see if the operational state thresholds are recovered, working in reverse order of checks
        execute_sql(cursor, "UPDATE HRI "
                            "SET RBSOperational = RBSOperational ^ 1, " \
                            "ErrorCode = NULL, "
                            "ErrorMessage = NULL, "
                            "LastUpdated = CURRENT_TIMESTAMP "
                            "FROM [dbo].[HRI_ACTIVATION_STATUS] HRI "
                            "JOIN " + table + " RATE "
                                              "ON HRI.HRI_ID = RATE.HRI_ID "
                                              "AND HRI.ErrorCode = ? "
                                              "AND HRI.RBSOperational = 0 AND RATE.MsgRate >= ? "
                                              "WHERE HRI.HRI_ID = ? " + topic,
                    code + 1, rate, hri)

        if cursor.rowcount > 0:
            send_event(hri, {
                'HRI': hri,
                'code': code,
                'message': ('RBS ' + chk +
                            ' messaging rate has been restored above the minimal operational threshold of ' +
                            rate + ' messages per second')
            })

            cursor.close()
            return


        # No changes for this check, just update the check-up time
        execute_sql(cursor, "UPDATE HRI "
                            "SET LastUpdated = CURRENT_TIMESTAMP "
                            "FROM [dbo].[HRI_ACTIVATION_STATUS] HRI "
                            "WHERE HRI.HRI_ID = ?", hri)
        cursor.close()

    except Exception as ex:
        logging.error(str(ex))

def rbs_check(conn: object, hri: int):
    logging.info("Checking communication status on HRI " + str(hri))

    check_rate_failed(conn, hri, 110, 'SPaT', 'J2735/SPAT')
    check_rate_failed(conn, hri, 120, 'MAP', 'J2735/MAP')
    check_rate_failed(conn, hri, 160, 'HRI plugin diagnostic', os.environ['HRI_STATUS_TOPIC'])
    check_rate_failed(conn, hri, 170, 'MAP plugin diagnostic', os.environ['MAP_STATUS_TOPIC'])
    check_rate_failed(conn, hri, 180, 'RSU immediate forward plugin diagnostic', os.environ['RSUIFM_STATUS_TOPIC'])

logging.basicConfig(format='%(levelname)s:%(message)s', level=logging.DEBUG)

logging.info("Initializing")

if '_TIMER_INIT' not in os.environ:
    os.environ['_TIMER_INIT'] = "1"

while True:
    # Retrieve the HRIs from the database
    try:
        conn = get_conn()
        cursor = conn.cursor()
        execute_sql(cursor, "SELECT DISTINCT HRI_ID "
                       "FROM [dbo].[HRI_ACTIVATION_STATUS] "
                       "WHERE (ErrorCode IS NULL OR ErrorCode <> 311) AND ("
                       "LastUpdated AT TIME ZONE 'UTC' < "
                       "DATEADD(MINUTE, -15, CURRENT_TIMESTAMP) "
                       "OR ? = 1)", int(os.environ['_TIMER_INIT']))

        for row in cursor.fetchall():
            logging.info("Detected new HRI to monitor: " + str(row[0]))
            threading.Thread(target = monitor_hri, args = [ int(row[0]) ]).start()

        os.environ['_TIMER_INIT'] = "0"

        cursor.close()
        conn.close()
    except Exception as ex:
        logging.error(str(ex))

    time.sleep(60)
