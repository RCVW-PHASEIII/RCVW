import azure.functions as func
import datetime
import json
import logging
import os
import pyodbc
import time
from azure.servicebus import ServiceBusClient, ServiceBusMessage

app = func.FunctionApp()

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

def schedule_hri(hri: int):
    time.sleep(0.5)
    send_message("cbs.scheduler.hri-check", hri, 'HRI', { 'HRI': hri })

def schedule_rbs(hri: int):
    time.sleep(0.5)
    send_message("cbs.scheduler.rbs-check", hri, 'RBS', { 'HRI': hri })

@app.service_bus_queue_trigger(arg_name="azservicebus",
                               queue_name="cbs.scheduler.hri-check",
                               connection="rcvwcbssb_CBSReceiver_SERVICEBUS")
def hri_check(azservicebus: func.ServiceBusMessage):
    req = json.loads(azservicebus.get_body().decode('utf-8'))

    # Make sure the message has HRI ID
    if 'HRI' not in req:
        logging.error('Scheduler message: ' + azservicebus.get_body().decode('utf-8') + ' invalid')
        return

    logging.info("Checking pre-emption status on HRI " + str(req['HRI']))
    try:
        # First, try to determine if HRI has been activated
        conn = get_conn()
        cursor = conn.cursor()
        cursor.execute("UPDATE HRI "
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
                           int(req['HRI']))

        if cursor.rowcount > 0:
            send_event(req['HRI'], {
                'HRI': req['HRI'],
                'active': True,
                'code': 1,
                'message': 'Preemption signal activated'
            })

            cursor.close()
            conn.close()
            schedule_hri(req['HRI'])
            return

        cursor.execute("UPDATE HRI "
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
                       int(req['HRI']))

        if cursor.rowcount > 0:
            send_event(req['HRI'], {
                'HRI': req['HRI'],
                'active': False,
                'code': 0,
                'message': 'Preemption signal deactivated'
            })

        cursor.close()
        conn.close()
    except Exception as ex:
        logging.error(str(ex))

    schedule_hri(req['HRI'])

def check_rate_failed(conn: object, req: str, code: int, chk: str, topic: str):
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
        conn = get_conn()
        cursor = conn.cursor()
        cursor.execute("UPDATE HRI "
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
                       code + 1, err, rate, int(req['HRI']))

        if cursor.rowcount > 0:
            send_event(req['HRI'], {
                'HRI': req['HRI'],
                'code': code + 1,
                'message': err
            })

            cursor.close()
            return

        # Next, see if the operational state thresholds are recovered, working in reverse order of checks
        cursor.execute("UPDATE HRI "
                       "SET RBSOperational = RBSOperational ^ 1, "\
                       "ErrorCode = NULL, "
                       "ErrorMessage = NULL, "
                       "LastUpdated = CURRENT_TIMESTAMP "
                       "FROM [dbo].[HRI_ACTIVATION_STATUS] HRI "
                       "JOIN " + table + " RATE "
                       "ON HRI.HRI_ID = RATE.HRI_ID "
                       "AND HRI.ErrorCode = ? "
                       "AND HRI.RBSOperational = 0 AND RATE.MsgRate >= ? "
                       "WHERE HRI.HRI_ID = ? " + topic,
                       code + 1, rate, int(req['HRI']))

        if cursor.rowcount > 0:
            send_event(req['HRI'], {
                'HRI': req['HRI'],
                'code': code,
                'message': ('RBS ' + chk +
                            ' messaging rate has been restored above the minimal operational threshold of ' +
                            rate + ' messages per second')
            })

            cursor.close()
            return


        # No changes for this check, just update the check-up time
        cursor.execute("UPDATE HRI "
                       "SET LastUpdated = CURRENT_TIMESTAMP "
                       "FROM [dbo].[HRI_ACTIVATION_STATUS] HRI "
                       "WHERE HRI.HRI_ID = ?", req['HRI'])
        cursor.close()

    except Exception as ex:
        logging.error(str(ex))

@app.service_bus_queue_trigger(arg_name="azservicebus",
                               queue_name="cbs.scheduler.rbs-check",
                               connection="rcvwcbssb_CBSReceiver_SERVICEBUS")
def rbs_check(azservicebus: func.ServiceBusMessage):
    req = json.loads(azservicebus.get_body().decode('utf-8'))

    # Make sure the message has HRI ID
    if 'HRI' not in req:
        logging.error('Scheduler message: ' + azservicebus.get_body().decode('utf-8') + ' invalid')
        return

    logging.info("Checking communication status on HRI " + str(req['HRI']))

    conn = get_conn()

    check_rate_failed(conn, req, 110, 'SPaT', 'J2735/SPAT')
    check_rate_failed(conn, req, 120, 'MAP', 'J2735/MAP')
    check_rate_failed(conn, req, 160, 'HRI plugin diagnostic', os.environ['HRI_STATUS_TOPIC'])
    check_rate_failed(conn, req, 170, 'MAP plugin diagnostic', os.environ['MAP_STATUS_TOPIC'])
    check_rate_failed(conn, req, 180, 'RSU immediate forward plugin diagnostic', os.environ['RSUIFM_STATUS_TOPIC'])

    conn.close()

    # Schedule next check
    schedule_rbs(req['HRI'])

# Need to start up the RSU checks upon initialization
@app.timer_trigger(schedule="10 0 * * * *", arg_name="myTimer", run_on_startup=True,
                   use_monitor=False)
def timer_trigger(myTimer: func.TimerRequest) -> None:
    if '_TIMER_INIT' not in os.environ:
        os.environ['_TIMER_INIT'] = "1"

    # Retrieve the HRIs from the database
    conn = get_conn()
    cursor = conn.cursor()
    cursor.execute("SELECT DISTINCT HRI_ID "
                       "FROM [dbo].[HRI_ACTIVATION_STATUS] "
                       "WHERE LastUpdated AT TIME ZONE 'UTC' < "
                       "DATEADD(MINUTE, -15, CURRENT_TIMESTAMP) "
                       "OR ? = 1", int(os.environ['_TIMER_INIT']))

    for row in cursor.fetchall():
        logging.info("Detected new HRI to monitor: " + str(row[0]))
        schedule_rbs(int(row[0]))
        schedule_hri(int(row[0]))

    os.environ['_TIMER_INIT'] = "0"

    cursor.close()
    conn.close()
