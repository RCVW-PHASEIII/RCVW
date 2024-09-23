console.log("Including tmx.vehicle.display.library.js")
var deviceName = "TMX";
var vehicleDisplayLibraryJsVersion = "3.3.0";
// Debug
var DebugOutput = false;
// EPCW variables
var EPCWTimerID = null;
var defaultEPCWTimout = 30000;
var EPCWActive = false;
var activeEPCWAlert = false;
var nearEPCWID = null;
var farEPCWID = null;
var leftEPCWID = null;
var rightEPCWID = null;
var nearEPCWSeverity = 0;
var farEPCWSeverity = 0;
var leftEPCWSeverity = 0;
var rightEPCWSeverity = 0;
var transitionRightEPCWFlag = false;
var transitionLeftEPCWFlag = false;
var transitionFarEPCWFlag = false;
var transitionNearEPCWFlag = false;
var activeDirections = [];
var activeNearEPCWImage = '../images/E-PCW/E-PCW_base.png';
var activeFarEPCWImage = '../images/E-PCW/E-PCW_base.png';
var activeLeftEPCWImage = '../images/E-PCW/E-PCW_base.png';
var activeRightEPCWImage = '../images/E-PCW/E-PCW_base.png';
var activeEPCWImage = '../images/E-PCW/E-PCW_base.png';
var baseEPCWImage = '../images/E-PCW/E-PCW_base.png';
var playEPCWAudio = false;
var baseImageActiveEPCW = false; 
var lastEPCWID = 0;
var VTRWActive = false;
var TSPWActive = false;
var CSWActive = false;

var defaultRCVWTimout = 10000;
var RCVWTimerID = null;
var RCVWSeverity = 0;
var RCVWActive =false;
var activeRCVWImage = '../images/RCVW/System Ready.png';
var baseRCVWImage = '../images/RCVW/System Ready.png';
var RCVWInterval = null;
var baseImageActiveRCVW = false; 
var activeRCVWAlert = false;
var RCVWActiveEvent = 0;
var RCVWCurrentEvent = 0;
var currentRCVWSeverity = 0;
var RCVWBlinkInterval = 300;
var RCVWErrorText = "";

//-- new RCVW
var RCVWAvailable = false;
var RCVWApproachInform = false;
var RCVWApproachWarning = false;
var RCVWHRIInform = false;
var RCVWHRIWarning = false;
var RCVWError = false;
var currentRCVWDisplay = 0;

var EPCWInterval = null;
var nextAudio = -1;

var cloakMode = false;

var appList = [];

var currAlert;
var appAlertsList = [];

function compareAppIds(json, alert) {
    if (json.AppId != 14 /* not RLVW */ && alert.AppId == 14 /* RLVW */) {
        return -1;
    } else if (json.AppId == 14 /* RLVW */ && alert.AppId != 14 /* not RLVW */) {
        return 1;
    } else if (json.AppId == 6 /* EPCW */ && alert.AppId != json.AppId /* not EPCW */) {
        return -1;
    } else if (json.AppId != 6 /* not EPCW */ && alert.AppId == 6 /* EPCW */) {
        return 1;
    } else if ((json.AppId == 8 /* TSPW */ || json.AppId == 13 /* TSPWPOV */) && (alert.AppId != 8 /* not TSPW */ && alert.AppId != 13 /* not TSPWPOV */)) {
        if (alert.AppId == 7 /* EVTRW */ && (TSPWActive && (json.EventCode == 20 || json.EventCode == 21))) {
            return -1;
        } else {
            return 1;
        }
    } else if ((json.AppId != 8 /* not TSPW */ && json.AppId != 13 /* not TSPWPOV */) && (alert.AppId == 8 /* TSPW */ || json.AppId == 13 /* TSPWPOV */)) {
        if (json.AppId == 7 /* EVTRW */ && (TSPWActive && (alert.EventCode == 20 || alert.EventCode == 21))) {
            return 1;
        } else {
            return -1;
        }
    }
    return 0;
}

function compareDistances(json, alert) {
    if (alert.AngleToRefPoint == json.AngleToRefPoint &&
        alert.AppId == json.AppId &&
        alert.CustomText == json.CustomText &&
        alert.DisplayDuration == json.DisplayDuration &&
        alert.DistanceToRefPoint == json.DistanceToRefPoint &&
        alert.EventCode == json.EventCode &&
        alert.EventID == json.EventID &&
        alert.InteractionId == json.InteractionId &&
        alert.Severity == json.Severity) {
        return 0;
    }
    if (alert.AngleToRefPoint <= 45) {
        if (json.AngleToRefPoint <= 45) {
            if (json.DistanceToRefPoint <= alert.DistanceToRefPoint) {
                return 1;
            } 
        } else if (json.AngleToRefPoint >= 345) {
            if (json.DistanceToRefPoint <= alert.DistanceToRefPoint) {
                return 1;
            } 
        }
    } else if (alert.AngleToRefPoint >= 345) {
        if (json.AngleToRefPoint >= 345) {
            if (json.DistanceToRefPoint <= alert.DistanceToRefPoint) {
                return 1;
            } else {
                if (json.AngleToRefPoint >= alert.AngleToRefPoint) {
                    return 1;
                }
            }
        } else if (json.AngleToRefPoint <= 45) {
            if (json.DistanceToRefPoint <= alert.DistanceToRefPoint) {
                return 1;
            } 
        }
    } else if (alert.AngleToRefPoint < 345 && alert.AngleToRefPoint > 45) {
        if (json.AngleToRefPoint >= 345 || json.AngleToRefPoint <= 45) {
            return 1;
        } else {
            if (alert.AngleToRefPoint >= 180 && json.AngleToRefPoint >= 180) {
                if (json.AngleToRefPoint > alert.AngleToRefPoint) {
                    return 1;
                } 
            } else if (alert.AngleToRefPoint >= 180 && json.AngleToRefPoint < 180) {
                if (json.AngleToRefPoint < (360 - alert.AngleToRefPoint)) {
                    return 1;
                } 
            } else if (alert.AngleToRefPoint < 180 && json.AngleToRefPoint >= 180) {
                if ((360 - json.AngleToRefPoint) < alert.AngleToRefPoint) {
                    return 1;
                } else if ((360 - json.AngleToRefPoint) == alert.AngleToRefPoint) {
                    if (json.DistanceToRefPoint <= alert.DistanceToRefPoint) {
                        return 1;
                    }
                }
            } else if (alert.AngleToRefPoint < 180 && json.AngleToRefPoint < 180) {
                if (json.AngleToRefPoint < alert.AngleToRefPoint) {
                    return 1;
                }
            }
        }
    }
    return -1;
}

function add_to_alert_list(json) {
    for (var i = 0; i < appAlertsList.length; i++) {
        var compareId = compareAppIds(json, appAlertsList[i]);
        if (compareId > 0) {
            if (compareDistances(json, appAlertsList[i]) > 0) {
                break;
            }
        } else if (compareId == 0) {
            appAlertsList.splice(i, 1, json);
            return;
        }
    }
    appAlertsList.splice(i, 0, json);
}

function appHandler(json_obj) {
    if (currAlert == null || currAlert == undefined) {
        displayAppMessage(json_obj);
        return;
    }

    var appExceedsFlag = compareAppIds(json_obj, currAlert);
    if (appExceedsFlag == 0) {
        displayAppMessage(json_obj);
    } else if (appExceedsFlag > 0) {
        if (compareDistances(json_obj, currAlert) > 0) {
            appAlertsList.splice(0, 0, currAlert);
            displayAppMessage(json_obj);
        } else if (json_obj.AppId == 14 || TSPWActive && json_obj.AppId == 7) {
            displayAppMessage(json_obj);
        }
    } else {
        add_to_alert_list(json_obj);
    }
}

function displayNextAlert() {
    currAlert = null;
    stopTimer();
    if (appAlertsList.length > 0) {
        var alert = appAlertsList.splice(0, 1);
        displayAppMessage(alert[0]);
    }
}

function createAppStatusMessageObject(appType)
{
    var appStatusMessage = new Object();
    appStatusMessage.appType = appType;
    appStatusMessage.active = false;
    appStatusMessage.activeEvent = null;
    appStatusMessage.severity = 0;
    // Display timer for image toggle
    appStatusMessage.displayTimer = 300;
    // Timeout interval
    appStatusMessage.interval = null;
    appStatusMessage.imageActive = null;
    appStatusMessage.imageInactive = null;
    appStatusMessage.imageDisplayed = false;

    return appStatusMessage;
}

function getAppObject(appType)
{
    var appObject = null;
    var tempList = appList;
    // Look for object
    for (var j = 0; j < tempList.length; j++) {
        var tempObj = tempList[j];
        if (appType.toUpperCase() == tempObj.appType.toUpperCase())
        {
            appObject = tempObj;
            break;
        }
    } 
    return appObject;
}

// Event listener from common.library 
addEventListener("newMessage", newMessageHandler, false);
addEventListener("disconnect", disconnectHandler, false);
addEventListener("connect", connectHandler, false);

// Handle connect
function connectHandler()
{
    if (DebugOutput) console.log("Connected Setting Background Image");
     $("#ActiveImage").attr('src', commonBackgroundImage);
}

function createAppStatusMessageObject(appType)
{
    var appStatusMessage = new Object();
    appStatusMessage.appType = appType;
    appStatusMessage.active = false;
    appStatusMessage.activeEvent = null;
    appStatusMessage.severity = 0;
    appStatusMessage.displayTimer = 300;

    return appStatusMessage;
}

// Receiver for unhandled messages from the common.library
function newMessageHandler(evt)
{

    console.log("Message: " + evt.detail.message);
    var json_obj = JSON.parse(evt.detail.message);
    if (json_obj.header.subtype === "Application") { 
        if (DebugOutput) console.log(evt.detail.message);
        handleAppMessage(json_obj.payload); 
    }
    else if (json_obj.header.subtype === "Basic") { 
        if (DebugOutput) console.log(evt.detail.message);
        handleAppMessage(json_obj.payload); 
    }    
    else if (json_obj.header.type === "__config") { 
        // Need to handle 
        if (json_obj.payload != undefined)
        {
            for (var i = 0; i < json_obj.payload.length; i++)
            {
                handleConfig(json_obj.payload[i]); 
            }
        }
    }
    else if (json_obj.header.subtype === "ModuleStatus") 
    {
        handleStatusMessage(json_obj.payload);
    }
//    else { console.log(timeStamp(evt.detail.time) + " Unknown Message: " + evt.detail.message);	}
}

// Handle disconnect
function disconnectHandler()
{
    appAlertsList = [];
    if (DebugOutput) console.log("Disconnected - Removing all images");
    clearEPCW();
    stopEPCWTimer();
    clearRCVW();
    TspwClearAllZones();
    stopSoundTimer();
    currAlert = null;
    $("#ActiveImage").removeClass("hidden");
    $("#CloakImage").addClass("hidden");
    $("#ActiveImage").attr('src', '../images/Common/Splash_Screen_2.png');
}

function displayAppMessage(json_obj) {
    // Do nothing if cloak, maintenance or degraded mode active
    if ((!screenNotification && !soundNotification) || (MaintenanceMode) || (OperationalModeDegraded)) return;

    TspwClearAllZones();
    currAlert = json_obj;

    // Set audio to no sound
    nextAudio = -1;
    if (json_obj.AppId == 6 /* EPCW */) {
        stopTimer();
        restartEPCWTimeout();
        playEPCWAudio = false;
        switch (json_obj.EventCode) {
            case "3": // Entered Area
                if (screenNotification) {
                    clearImage();
                    populateHisImageMessageObject(6, json_obj.Id, json_obj.Severity);
                    setImage('../images/E-PCW/E-PCW_base.png', "");
                }
                currentAudio = -1;
                break;
            case "4": // Exited Area
                stopEPCWTimer();
                if (screenNotification) {
                    populateHisImageMessageObject(6, json_obj.Id, json_obj.Severity);
                }
                clearEPCW();
                currentAudio = -1;
                break;
            case "5": // Detected Area
                // No Action
                break;
            case "6": // Near side Pedestrian
                if (nearEPCWSeverity != 0) {
                    playEPCWAudio = true;
                }
                if (screenNotification) {
                    populateHisImageMessageObject(6, json_obj.Id, json_obj.Severity);
                    displayEPCW();
                }
                break;
            case "7": // Far side Pedestrian
                if (farEPCWSeverity != 0) {
                    playEPCWAudio = true;
                }
                if (screenNotification) {
                    populateHisImageMessageObject(6, json_obj.Id, json_obj.Severity);
                    displayEPCW();
                }
                break;
            case "8": // Left side Pedestrian
                if (leftEPCWSeverity != 0) {
                    playEPCWAudio = true;
                }
                if (screenNotification) {
                    populateHisImageMessageObject(6, json_obj.Id, json_obj.Severity);
                    displayEPCW();
                }
                break;
            case "9": // Right side Pedestrian
                if (rightEPCWSeverity != 0) {
                    playEPCWAudio = true;
                }
                if (screenNotification) {
                    populateHisImageMessageObject(6, json_obj.Id, json_obj.Severity);
                    displayEPCW();
                }
                break;
            case "20": // Pedestrian alert complete 
                //if (lastEPCWID == json_obj.EventID) {
                stopAudio();
                //}
                if (screenNotification) {
                    populateHisImageMessageObject(6, json_obj.Id, json_obj.Severity);
                    displayEPCW();
                }
                break;
            case "21": // Detected Area
                if (screenNotification && !EPCWActive) {
                    clearImage();
                    populateHisImageMessageObject(6, json_obj.Id, json_obj.Severity);
                    setImage('../images/E-PCW/E-PCW_base.png', "");
                }
                // No Action
                break;
        }
    } else if (json_obj.AppId == 7 /* EVTRW */) {
        switch (json_obj.EventCode) {
            case "1": // Entered Area
                if (screenNotification) {
                    populateHisImageMessageObject(7, json_obj.Id, json_obj.Severity);
                    commonBackgroundImage = "../images/VTRW/VTRW_base.png";
                    clearImage();
                    setImage(commonBackgroundImage, "");
                }
                //setImage('../images/VTRW/VTRW_base.png', "");
                break;
            case "2": // Exited Area
                if (screenNotification) {
                    populateHisImageMessageObject(7, json_obj.Id, json_obj.Severity);
                    commonBackgroundImage = "../images/RCVW/System Ready.png";
                    clearImage();
                    setImage(commonBackgroundImage, "");
                    displayNextAlert();
                }
                break;
            case "13":  // Ahead Left
                if (VTRWActive && json_obj.Severity == "0") {
                    if (screenNotification) {
                        populateHisImageMessageObject(7, json_obj.Id, json_obj.Severity);
                        setImage('../images/VTRW/VTRW_base.png', "");
                    }
                    break;
                }
            case "17":	// Behind Left
                if (VTRWActive && json_obj.Severity == "1") {
                    if (screenNotification) {
                        populateHisImageMessageObject(7, json_obj.Id, json_obj.Severity);
                        setTimedImage('../images/VTRW/VTRW_caution_left_2.png', json_obj.CustomText, json_obj.DisplayDuration);
                    }
                    if (VTRWSeverity <= 1) {
                        nextAudio = 6;
                    }
                }
                break;
            case "12": // Vehicle ahead
                if (VTRWActive && json_obj.Severity == "2") {
                    if (screenNotification) {
                        populateHisImageMessageObject(7, json_obj.Id, json_obj.Severity);
                        setTimedImage('../images/VTRW/VTRW_warning_left_1.png', json_obj.CustomText, json_obj.DisplayDuration);
                    }
                    if (VTRWSeverity <= 2) {
                        nextAudio = 7;
                    }
                }
                break;
            default:
                break;
        }
    } else if (json_obj.AppId == 8 /* TSPW */ || json_obj.AppId == 13 /* TSPWPOV */) {
        TspwDisplayAlert(json_obj);
    } else if (json_obj.AppId == 14 /* RLVW */) {
        switch (json_obj.EventCode) {
            // TODO: Test logic for prioritization; will need to replace with actual
            case "36":
                if (json_obj.Severity == "0") {
                    populateHisImageMessageObject(14, json_obj.Id, json_obj.Severity);
                    commonBackgroundImage = "../images/RCVW/System Ready.png";
                    clearImage();
                    setImage(commonBackgroundImage, "");
                    displayNextAlert();
                    nextAudio = -1;
                } else if (json_obj.Severity == "1") {
                    if (screenNotification) {
                        populateHisImageMessageObject(14, json_obj.Id, json_obj.Severity);
                        commonBackgroundImage = "../images/RLVW/04-rlvw-alert-alert.png";
                        clearImage();
                        setImage(commonBackgroundImage, "");
                    }
                    nextAudio = 26;
                } else if (json_obj.Severity == "2") {
                    if (screenNotification) {
                        populateHisImageMessageObject(14, json_obj.Id, json_obj.Severity);
                        commonBackgroundImage = "../images/RLVW/03-rlvw-alert.png";
                        clearImage();
                        setImage(commonBackgroundImage, "");
                    }
                    nextAudio = 25;
                }
                break;
        }
    }

    // If audio needs to be changed play audio
    if (nextAudio != -1 && soundNotification) {
        playAudio(nextAudio, audioFiles);
    }
}

/*
This function handles all of the Application:Application messages and displays the appropriate 
Image.
*/
function handleAppMessage(json_obj)
{
    // Do nothing if cloak, maintenance or degraded mode active
    if ((!screenNotification && !soundNotification) || (MaintenanceMode) || (OperationalModeDegraded)) return;

    // Set audio to no sound
    nextAudio = -1;
    if (json_obj.AppId == 2) // FCW
    {
        switch (json_obj.Severity) {
        case "0":
            //            setTimedImage('../images/FCW/3-base-art.png', json_obj.CustomText,  json_obj.DisplayDuration);
            break;
        case "1":
            if (screenNotification) {
                setTimedImage('../images/FCW/4-vehicle-ahead-alert-yellow.png', json_obj.CustomText, json_obj.DisplayDuration);
            }
            if (FCWSeverity < 1)
            {
                if (screenNotification) {
                    populateHisImageMessageObject(2, json_obj.Id, json_obj.Severity);
                }
                FCWSeverity = 1;
                // TODO Change
                nextAudio = 4;
            }
            break;
        case "2":
            if (screenNotification) {
                setTimedImage('../images/FCW/5-vehicle-ahead-warning-red.png', json_obj.CustomText, json_obj.DisplayDuration);
            }
            if (FCWSeverity < 2)
            {
                if (screenNotification) {
                    populateHisImageMessageObject(2, json_obj.Id, json_obj.Severity);
                }
                FCWSeverity = 2;
                nextAudio = 8;
            }
            break;
        default:
                if (screenNotification) {
                    clearImage();
                    setImage(commonBackgroundImage, "");
                }
            break;
        }
    }
    else if (json_obj.AppId == 3) // EEBL
    {
        switch (json_obj.Severity) {
        case "0":
            if (screenNotification) {
                setTimedImage('../images/FCW/3-base-art.png', json_obj.CustomText, json_obj.DisplayDuration);
                populateHisImageMessageObject(3, json_obj.Id, json_obj.Severity);
            }
            break;
        case "1":
        case "2":
            if (screenNotification) {
                setTimedImage('../images/EEBL/4-eebl.png', json_obj.CustomText, json_obj.DisplayDuration);
            }
            if (EEBLSeverity < 2)
            {
                if (screenNotification) {
                    populateHisImageMessageObject(2, json_obj.Id, json_obj.Severity);
                }
                EEBLSeverity = 2;
                nextAudio = 25;
            }            
            break;
        default:
            if (screenNotification) {
                clearImage();
                setImage(commonBackgroundImage, "");
            }
            break;
        }
    }
    else if (json_obj.AppId == 6) // EPCW
    {
        switch (json_obj.EventCode) {
        case "3": // Entered Area
            EPCWActive = true;
            break;
        case "4": // Exited Area
            EPCWActive = false;
            break;
        case "5": // Detected Area
            // No Action
            break;
        case "6": // Near side Pedestrian
            if (json_obj.EventID != nearEPCWID) {
                transitionNearEPCWFlag = true;
            }
            nearEPCWID = json_obj.EventID;
            lastEPCWID = json_obj.EventID;
            if (nearEPCWSeverity != json_obj.Severity) {
                nearEPCWSeverity = json_obj.Severity;
                if (nearEPCWSeverity != 0) {
                    //playEPCWAudio = true;
                }
            }
            break;
        case "7": // Far side Pedestrian
            if (json_obj.EventID != farEPCWID) {
                transitionFarEPCWFlag = true;
            }
            farEPCWID = json_obj.EventID;
            lastEPCWID = json_obj.EventID;
            if (farEPCWSeverity != json_obj.Severity) {
                farEPCWSeverity = json_obj.Severity;
                if (farEPCWSeverity != 0) {
                    //playEPCWAudio = true;
                }
            }
            break;
        case "8": // Left side Pedestrian
            if (json_obj.EventID != leftEPCWID) {
                transitionLeftEPCWFlag = true;
            }
            leftEPCWID = json_obj.EventID;
            lastEPCWID = json_obj.EventID;
            if (leftEPCWSeverity != json_obj.Severity) {
                leftEPCWSeverity = json_obj.Severity;
                if (leftEPCWSeverity != 0) {
                    //playEPCWAudio = true;
                }
            }
            break;
        case "9": // Right side Pedestrian
            if (json_obj.EventID != rightEPCWID) {
                transitionRightEPCWFlag = true;
            }
            rightEPCWID = json_obj.EventID;
            lastEPCWID = json_obj.EventID;
            if (rightEPCWSeverity != json_obj.Severity) {
                rightEPCWSeverity = json_obj.Severity;
                if (rightEPCWSeverity != 0) {
                    //playEPCWAudio = true;
                }
            }
            break;
        case "20": // Pedestrian alert complete    
            if (currAlert == null) return;
            if (currAlert.AppId == null || currAlert.AppId == undefined) {
                return;
            }
            var src = currAlert.AppId;
            if (json_obj.EventID == nearEPCWID) {
                if (src != 6) {
                    transitionNearEPCWFlag = false;
                } else {
                    transitionNearEPCWFlag = true;
                }
                nearEPCWID = null;
                nearEPCWSeverity = 0;
            } else if (json_obj.EventID == farEPCWID) {
                if (src != 6) {
                    transitionFarEPCWFlag = false;
                } else {
                    transitionFarEPCWFlag = true;
                }
                farEPCWID = null;
                farEPCWSeverity = 0;
            } else if (json_obj.EventID == leftEPCWID) {
                if (src != 6) {
                    transitionLeftEPCWFlag = false;
                } else {
                    transitionLeftEPCWFlag = true;
                }
                leftEPCWID = null;
                leftEPCWSeverity = 0;
            } else if (json_obj.EventID == rightEPCWID) {
                if (src != 6) {
                    transitionRightEPCWFlag = false;
                } else {
                    transitionRightEPCWFlag = true;
                }
                rightEPCWID = null;
                rightEPCWSeverity = 0;
            }
            break;
        case "21": // Detected Area
            if (!EPCWActive) {
                EPCWActive = true;
            }
            // No Action
            break;
        }
        appHandler(json_obj);
    }
    else if (json_obj.AppId == 7) // EVTRW
    {
        switch (json_obj.EventCode) {
        case "1": // Entered Area
            VTRWActive = true;
            break;
        case "2": // Exited Area
        	VTRWActive = false;
            break;
        case "13":  // Ahead Left
        	if (VTRWActive && json_obj.Severity == "0") {
        	    VTRWSeverity = 0;
        	}
        	break;
        case "17":	// Behind Left
        	if (VTRWActive && json_obj.Severity == "1") {
        		if (VTRWSeverity < 1) {
        			VTRWSeverity = 1;
        		}
        	}
            break;
        case "12": // Vehicle ahead
        	if (VTRWActive && json_obj.Severity == "2") {
        		if (VTRWSeverity < 2) {
        			VTRWSeverity = 2;
        		}
        	}
        	break;
       default:
    	   	break;
        }
        appHandler(json_obj);
    }
    else if (json_obj.AppId == 1) // CSW
    {
        // TODO: Need to add a display function
        if (DebugOutput) console.log("CSW Event Code " + json_obj.EventCode);
        switch (json_obj.EventCode) {
        case "0": // Alert
            switch (json_obj.Severity) {
            case "0": // Alert
                if (screenNotification) {
                    setTimedImage('../images/CSW/CSW_info.png', json_obj.CustomText, json_obj.DisplayDuration);
                }
                break;
            case "1": // Alert
                if (screenNotification) {
                    setTimedImage('../images/CSW/CSW_alert.png', json_obj.CustomText, json_obj.DisplayDuration);
                }
                if (CSWSeverity != "1")
                {
                    if (CSWSeverity < 1)
                    {
                        CSWSeverity = 1;
                        currentAudio = -1;
                        nextAudio = 25;
                    }
                }
                break;
            case "2": // Alert
                if (screenNotification) {
                    setTimedImage('../images/CSW/CSW_warning.png', json_obj.CustomText, json_obj.DisplayDuration);
                }
                if (CSWSeverity != "2")
                {
                    if (CSWSeverity < 2)
                    {
                        CSWSeverity = 2;
                        currentAudio = -1;
                        nextAudio = 8;
                    }
                }
                break;
            }
            break;
        case "1": // Entered Area
            if (!CSWActive)
            {
                CSWActive = true;
                if (screenNotification) {
                    clearImage();
                    populateHisImageMessageObject(1, json_obj.Id, json_obj.Severity);
                    setImage('../images/CSW/CSW_base.png', "");
                }
            }
            break;
        case "2": // Exited Area
            CSWActive = false;
            CSWSeverity = 0;
            if (screenNotification) {
                clearImage();
                setImage(commonBackgroundImage, "");
            }
            break;
       default:
            break;
        }
    }
    else if (json_obj.AppId == 10) // RCVW
    {

        console.log("GOT RCVW EventCode " + json_obj.EventCode);
        // Check if RCVW object exists, if not create
        var tempObject = getAppObject("RCVW");
        if (tempObject == null)
        {

        }
        switch (json_obj.EventCode) {
        case "41": // Entered Area
			severity = Number(json_obj.Severity);
			if (severity)
			{
				RCVWAvailable = true;
			}
			else {
				// Exited Area
				RCVWAvailable = false;
			}
            break;
        case "42": // Approach Inform
			severity = Number(json_obj.Severity);
			if (severity) {
				RCVWApproachInform = true;
			}
			else {
				RCVWApproachInform = false;
			}
			break;
        case "43": // Approach Warning
			severity = Number(json_obj.Severity);
			if (severity) {
				RCVWApproachWarning = true;
			}
			else {
				RCVWApproachWarning = false;
			}
			break;
        case "44": // HRI Warning
			severity = Number(json_obj.Severity);
			if (severity) {
                RCVWHRIInform = (severity == 1);
				RCVWHRIWarning = !RCVWHRIInform;
			}
			else {
                RCVWHRIInform = false;
				RCVWHRIWarning = false;
			}
			break;
        case "45": // Error
			severity = Number(json_obj.Severity);
			RCVWErrorText = json_obj.CustomText;
			if (severity) {
				RCVWError = true;
			}
			else {
				RCVWError = false;
			}
			break;
       default:
            if (DebugOutput) console.log("Invalid Event Code for RCVW - " + json_obj.EventCode);
            break;
        }
		 displayRCVW();
    } else if (json_obj.AppId == 8 || // TSPW
             json_obj.AppId == 13)  {// TSPWPOV
        TspwHandleAlert(json_obj);
        appHandler(json_obj);
    } else if (json_obj.AppId == 14) {  
        appHandler(json_obj);
    }
    
    // If audio needs to be changed play audio
	if (DebugOutput) console.log("Next audio:" + nextAudio)
    if (nextAudio != -1 && soundNotification) playAudio(nextAudio, audioFiles);
}

function getDisplay()
{
	var newRCVWDisplay = 0;
	if (RCVWError) {
		newRCVWDisplay = 45;
	}
	else if (RCVWHRIWarning || RCVWHRIInform) {
		newRCVWDisplay = 44;
	}
	else if (RCVWApproachWarning) {
		newRCVWDisplay = 43;
	}
	else if (RCVWApproachInform) {
		newRCVWDisplay = 42;
	}
	else if (RCVWAvailable) {
		newRCVWDisplay = 41;
	}
	return  newRCVWDisplay;

}

function displayRCVW()
{
	var newDisplay = getDisplay();
    var currentActiveRCVWImage = activeRCVWImage;
    var currentRCVWBlinkInterval = RCVWBlinkInterval;

	if (newDisplay != currentRCVWDisplay)
	{

       switch (newDisplay) {
		 case 0:
			activeRCVWAlert = false;
            stopRCVWDisplay();
            document.getElementById("bottomDiv").innerHTML = "";
            $("#ActiveImage").attr('src', '../images/RCVW/System Ready.png');
            break;
		 case 41: // Available
			activeRCVWAlert = false;
            stopRCVWDisplay();
            document.getElementById("bottomDiv").innerHTML = "";
			 $("#ActiveImage").attr('src', '../images/RCVW/System Available.png');
            activeRCVWImage = '../images/RCVW/System Available.png';
            RCVWBlinkInterval = -1;
            break;
		 case 42: // Approach Inform
			activeRCVWAlert = true;
			stopRCVWDisplay();
			document.getElementById("bottomDiv").innerHTML = "";
				$("#ActiveImage").attr('src', '../images/RCVW/Approach Inform.png');
			activeRCVWImage = '../images/RCVW/Approach Inform.png';
			RCVWBlinkInterval = -1;
			if (currentRCVWDisplay < newDisplay) {
				nextAudio = 30;
			}

            break;
		 case 43: // Approach Warning
			activeRCVWAlert = true;
			stopRCVWDisplay();
			document.getElementById("bottomDiv").innerHTML = "";
            activeRCVWImage = '../images/RCVW/Approach Warning 1.png';
            baseRCVWImage = '../images/RCVW/Approach Warning 2.png';
            RCVWBlinkInterval = 500;
			startRCVWDisplay();
			if (currentRCVWDisplay < newDisplay) {
				nextAudio = 31;
			}
            break;
		 case 44: // HRI Warning
			activeRCVWAlert = true;
			stopRCVWDisplay();
			document.getElementById("bottomDiv").innerHTML = "";
            if (RCVWHRIInform) {
                activeRCVWImage = '../images/RCVW/Clear HRI Warning 1.png';
                baseRCVWImage = '../images/RCVW/Clear HRI Warning 2.png';
            } else {
                activeRCVWImage = '../images/RCVW/Clear HRI Active Warning 1.png';
                baseRCVWImage = '../images/RCVW/Clear HRI Active Warning 2.png';
            }
            RCVWBlinkInterval = 500;
			startRCVWDisplay();
			if (currentRCVWDisplay < newDisplay) {
				nextAudio = 31;
			}
            break;
		 case 45: // Error
			activeRCVWAlert = true;
			stopRCVWDisplay();
			document.getElementById("bottomDiv").innerHTML = RCVWErrorText;
			$("#ActiveImage").attr('src', '../images/RCVW/System Unavailable.png');
			activeRCVWImage = '../images/RCVW/System Unavailable.png';
			RCVWBlinkInterval = -1;
			if (currentRCVWDisplay < newDisplay) {
				nextAudio = 30;
			}

            break;
		}
		currentRCVWDisplay = newDisplay;
	}


}
// ------- RCVW ----------------------------------------------------------
function toggleRCVWImage()
{
//    if ((!screenNotification && !soundNotification) && (!MaintenanceMode) && (!OperationalModeDegraded))
//    {
        if (baseImageActiveRCVW) {
			$("#ActiveImage").attr('src', activeRCVWImage);
		}
        else 
        {
            $("#ActiveImage").attr('src', baseRCVWImage);
        }
        baseImageActiveRCVW = !baseImageActiveRCVW;
//    }
}

function startRCVWDisplay()
{
    baseImageActiveRCVW = true;
    toggleRCVWImage();
    RCVWInterval = setInterval(toggleRCVWImage, RCVWBlinkInterval);
}

function restartRCVWDisplay()
{
    clearInterval(RCVWInterval);
    RCVWInterval = null;
    RCVWInterval = setInterval(toggleRCVWImage, RCVWBlinkInterval);
}

function stopRCVWDisplay()
{
    stopRCVWWTimer();
    clearInterval(RCVWInterval);
    RCVWInterval = null;
    activeRCVWImage = '../images/RCVW/System Ready.png';
    activeRCVWAlert = false;
    if (screenNotification) {
        $("#ActiveImage").attr('src', activeRCVWImage);
    }
}

function clearRCVW()
{
    stopRCVWDisplay();
    if (screenNotification) {
        clearImage();
        his.payload.HISPreState = activeRCVWImage;
        his.payload.HISPostState = commonBackgroundImage;
        sendCommand("TEST;" + JSON.stringify(his));
    }
}

function startRCVWTimer(duration)
{
    RCVWTimerID = setTimeout(RCVWTimeout, duration);
}

function restartRCVWTimeout()
{
    stopRCVWTimer();
    startRCVWTimer(defaultRCVWTimout);
}

function stopRCVWWTimer()
{
    if (RCVWTimerID != null)
    {
        clearTimeout(RCVWTimerID);
        RCVWTimerID = null;
    }    
}
function RCVWTimeout()
{
    if (DebugOutput) console.log("RCVW Timerout Clearing");
    clearRCVW();
}

// ------- RCVW ----------------------------------------------------------


function displayEPCW()
{
    var audioPlayIndex = -1;
    var preRightState = activeRightEPCWImage, preLeftState = activeLeftEPCWImage, preNearState = activeNearEPCWImage, preFarState = activeFarEPCWImage;
    activeDirections = [];
    if ((nearEPCWID!=null) || (farEPCWID!=null) || (leftEPCWID!=null) || (rightEPCWID!=null)) {
        if (nearEPCWID != null) {
            if (nearEPCWSeverity >= 1 && nearEPCWSeverity <= 2) {
                activeDirections.push("NearDirectionImage");
                if (nearEPCWSeverity == 1) {
                    $("#NearDirectionImage").attr('src', "../images/E-PCW/E-PCW_caution_near.png");
                    activeNearEPCWImage = "../images/E-PCW/E-PCW_caution_near.png";
                } else if (nearEPCWSeverity == 2) {
                    $("#NearDirectionImage").attr('src', "../images/E-PCW/E-PCW_warning_near.png");
                    activeNearEPCWImage = "../images/E-PCW/E-PCW_warning_near.png";
                }
            }
        } else {
            activeNearEPCWImage = '../images/E-PCW/E-PCW_base.png';
        }

        if (rightEPCWID != null) {
            if (rightEPCWSeverity >= 1 && rightEPCWSeverity <= 2) {
                activeDirections.push("RightDirectionImage");
                if (rightEPCWSeverity == 1) {
                    $("#RightDirectionImage").attr('src', "../images/E-PCW/E-PCW_caution_right.png");
                    activeRightEPCWImage = "../images/E-PCW/E-PCW_caution_right.png";
                } else if (rightEPCWSeverity == 2) {
                    $("#RightDirectionImage").attr('src', "../images/E-PCW/E-PCW_warning_right.png");
                    activeRightEPCWImage = "../images/E-PCW/E-PCW_warning_right.png";
                }
            }
        } else {
            activeRightEPCWImage = '../images/E-PCW/E-PCW_base.png';
        }
        
        if (farEPCWID != null) {
            if (farEPCWSeverity >= 1 && farEPCWSeverity <= 2) {
                activeDirections.push("FarDirectionImage");
                if (farEPCWSeverity == 1) {
                    $("#FarDirectionImage").attr('src', "../images/E-PCW/E-PCW_caution_far.png");
                    activeFarEPCWImage = "../images/E-PCW/E-PCW_caution_far.png";
                } else if (farEPCWSeverity == 2) {
                    $("#FarDirectionImage").attr('src', "../images/E-PCW/E-PCW_warning_far.png");
                    activeFarEPCWImage = "../images/E-PCW/E-PCW_warning_far.png";
                }
            }
        } else {
            activeFarEPCWImage = '../images/E-PCW/E-PCW_base.png';
        }
        
        if (leftEPCWID != null) {
            if (leftEPCWSeverity >= 1 && leftEPCWSeverity <= 2) {
                activeDirections.push("LeftDirectionImage");
                if (leftEPCWSeverity == 1) {
                    $("#LeftDirectionImage").attr('src', "../images/E-PCW/E-PCW_caution_left.png");
                    activeLeftEPCWImage = "../images/E-PCW/E-PCW_caution_left.png";
                } else if (leftEPCWSeverity == 2) {
                    $("#LeftDirectionImage").attr('src', "../images/E-PCW/E-PCW_warning_left.png");
                    activeLeftEPCWImage = "../images/E-PCW/E-PCW_warning_left.png";
                }
            }
        } else {
            activeLeftEPCWImage = '../images/E-PCW/E-PCW_base.png';
        }
        
        if ((nearEPCWID!=null) && ((rightEPCWID!=null) || (leftEPCWID!=null) || (farEPCWID!=null))) {
            if (rightEPCWID!=null) {
                if ((nearEPCWSeverity == 1) && (rightEPCWSeverity == 1)) {
                    audioPlayIndex = 15;
                }
                else {
                    if ((nearEPCWSeverity == 1) && (rightEPCWSeverity == 2)) audioPlayIndex = 23;           // No Sound File
                    else if ((nearEPCWSeverity == 2) && (rightEPCWSeverity == 1)) audioPlayIndex = 19;      // no Sound File
                    else if ((nearEPCWSeverity == 2) && (rightEPCWSeverity == 2)) audioPlayIndex = 22;      
                }
            }
            else if (leftEPCWID!=null){
                if ((nearEPCWSeverity == 1) && (leftEPCWID == 1)) {
                    audioPlayIndex = 14;
                }
                else {
                    if ((nearEPCWSeverity == 1) && (leftEPCWID == 2)) audioPlayIndex = 18;              // No Sound File
                    else if ((nearEPCWSeverity == 2) && (leftEPCWID == 1)) audioPlayIndex = 19;         // No Sound File
                    else if ((nearEPCWSeverity == 2) && (leftEPCWID == 2)) audioPlayIndex = 21;         // No Sound File
                }
            }
            else if (farEPCWID!=null) {
                if ((nearEPCWSeverity == 1) && (farEPCWID == 1)) {
                    audioPlayIndex = 13;
                }
                else {
                    if ((nearEPCWSeverity == 1) && (farEPCWID == 2)) audioPlayIndex = 17;               // No sound File
                    else if ((nearEPCWSeverity == 2) && (farEPCWID == 1)) audioPlayIndex = 20;  
                    else if ((nearEPCWSeverity == 2) && (farEPCWID == 2)) audioPlayIndex = 22;          
                }
            }
        }
        else if (nearEPCWID!=null) {
            if (nearEPCWSeverity == 1) {
                audioPlayIndex = 12;
            }
            else if (nearEPCWSeverity == 2) {
                audioPlayIndex = 19;
            }
        }
        else if (rightEPCWID!=null) {
            if (rightEPCWSeverity == 1) {
                audioPlayIndex = 16;
            }
            else if (rightEPCWSeverity == 2) {      
                audioPlayIndex = 23;
            }
        }
        else if (farEPCWID!=null) {
            if (farEPCWSeverity == 1) {     
                audioPlayIndex = 10;
            }
            else if (farEPCWSeverity == 2) {
                audioPlayIndex = 17;
            }
        }
        else if (leftEPCWID!=null) {
            if (leftEPCWSeverity == 1) {
                audioPlayIndex = 11;
            }
            else if (leftEPCWSeverity == 2) {
                audioPlayIndex = 18;
            }
        }

//        console.log("Play audio:" + playEPCWAudio + " index:" + audioPlayIndex);
        if ((playEPCWAudio) && (audioPlayIndex != -1)) {
            playAudio(audioPlayIndex, audioFiles);
        }

        if (!activeEPCWAlert) {
            activeEPCWAlert = true;
            startEPCWDisplay();
        }

        if (transitionRightEPCWFlag) {
        	his.payload.HISPreState = preRightState;
        	his.payload.HISPostState = activeRightEPCWImage;
            if (DebugOutput) console.log("displayEPCW transitionRightEPCWFlag :" + JSON.stringify(his));

            sendCommand("TEST;" + JSON.stringify(his));
            transitionRightEPCWFlag = false;
        }
        if (transitionLeftEPCWFlag) {
            his.payload.HISPreState = preLeftState;
            his.payload.HISPostState = activeLeftEPCWImage;
            if (DebugOutput) console.log("displayEPCW transitionLeftEPCWFlag :" + JSON.stringify(his));
            sendCommand("TEST;" + JSON.stringify(his));
            transitionLeftEPCWFlag = false;
        }
        if (transitionFarEPCWFlag) {
            his.payload.HISPreState = preFarState;
            his.payload.HISPostState = activeFarEPCWImage;
            if (DebugOutput) console.log("displayEPCW transitionFarEPCWFlag :" + JSON.stringify(his));
            sendCommand("TEST;" + JSON.stringify(his));
            transitionFarEPCWFlag = false;
        }
        if (transitionNearEPCWFlag) {
            his.payload.HISPreState = preNearState;
            his.payload.HISPostState = activeNearEPCWImage;
            if (DebugOutput) console.log("displayEPCW transitionNearEPCWFlag :" + JSON.stringify(his));
            sendCommand("TEST;" + JSON.stringify(his));
            transitionNearEPCWFlag = false;
        }

        $("#ActiveImage").attr('src', baseEPCWImage);
    }
    else {
        if (activeEPCWAlert == true) {
            if (transitionRightEPCWFlag) {
                his.payload.HISPreState = activeRightEPCWImage; 
                transitionRightEPCWFlag = false;
            } else if (transitionLeftEPCWFlag) {
                his.payload.HISPreState = activeLeftEPCWImage;
                activeLeftEPCWImage = '../images/E-PCW/E-PCW_base.png';
                transitionLeftEPCWFlag = false;
            } else if (transitionFarEPCWFlag) {
                his.payload.HISPreState = activeFarEPCWImage;
                activeFarEPCWImage = '../images/E-PCW/E-PCW_base.png';
                transitionFarEPCWFlag = false;
            } else if (transitionNearEPCWFlag) {
                his.payload.HISPreState = activeNearEPCWImage;
                activeNearEPCWImage = '../images/E-PCW/E-PCW_base.png';
                transitionNearEPCWFlag = false;
            }

            his.payload.HISPostState = '../images/E-PCW/E-PCW_base.png';
           if (DebugOutput) console.log("displayEPCW activeEPCWAlert :" + JSON.stringify(his));

            sendCommand("TEST;" + JSON.stringify(his));
        }
        stopEPCWDisplay();
        currentAudio = -1;
        activeEPCWAlert = false;
    }
}

function toggleEPCWImage()
{
    $("[id$=DirectionImage]").addClass("hidden");

    if ((screenNotification) && (!MaintenanceMode) && (!OperationalModeDegraded))
    {
        for (var i = 0; i < activeDirections.length; i++) {
            var direction = $("#" + activeDirections[i]);
            if (baseImageActiveEPCW) {
               direction.removeClass("hidden");
            } else {
               direction.addClass("hidden");
            }
        }
        baseImageActiveEPCW = !baseImageActiveEPCW;
    }

}

function startEPCWDisplay()
{
    baseImageActiveEPCW = true;
    toggleEPCWImage();
    EPCWInterval = setInterval(toggleEPCWImage, 300);
}

function stopEPCWDisplay()
{
    clearInterval(EPCWInterval);
    EPCWInterval = null;
    activeEPCWImage = '../images/E-PCW/E-PCW_base.png';
    if (screenNotification) {
        $("#ActiveImage").attr('src', activeEPCWImage);
    }
    $("[id$=DirectionImage]").addClass("hidden");
}

function clearEPCW()
{
    stopEPCWDisplay();
    nearEPCWID = null;
    farEPCWID = null;
    leftEPCWID = null;
    rightEPCWID = null;
    nearEPCWSeverity = null;
    farEPCWSeverity = null;
    leftEPCWSeverity = null;
    rightEPCWSeverity = null;
    transitionRightEPCWFlag = false;
    transitionLeftEPCWFlag = false;
    transitionFarEPCWFlag = false;
    transitionNearEPCWFlag = false;
    if (screenNotification) {

        clearImage();
        setImage(commonBackgroundImage, "");
    }
    displayNextAlert();
}

function startEPCWTimer(duration)
{
    EPCWTimerID = setTimeout(EPCWTimeout, duration);
}

function restartEPCWTimeout()
{
    stopEPCWTimer();
    startEPCWTimer(defaultEPCWTimout);
}

function stopEPCWTimer()
{
    if (EPCWTimerID != null)
    {
        clearTimeout(EPCWTimerID);
        EPCWTimerID = null;
    }    
}
function EPCWTimeout()
{
/*
    console.log("EPCW Timerout Clearing");
    clearEPCW();
    EPCWActive = false;
    displayNextAlert();
*/
    if (DebugOutput) console.log("EPCW Timerout Clearing");
    EPCWActive = false;
    stopEPCWTimer();
    clearEPCW();
    currentAudio = -1;
    displayNextAlert();
}


// ------- EPCW ----------------------------------------------------------

// ------- TSPW Start ----------------------------------------------------
var tspwPedestrianTimout_ms = 2000;
var tspwValidSeverity = {"1": true, "2": true};

var tspwZoneInfoTable = {
	"10": { 		// event Code
		zoneName: "OnCurbsidePed",
		timerId: null,
		currentAudIdx: -1,
		files: {
		"1":{   	// severity
				imgFile: ["../images/TVO/05-inform-zone2.png", ""],
				audFileIdx: [27, -1]
			},
		"2":{
				imgFile: ["../images/TVO/05-inform-zone2.png", ""], // no warning image present
				audFileIdx: [27, -1]
			}}},		
	"32": {
		zoneName: "InRoadwayFwdCenterPed",
		timerId: null,
		currentAudIdx: -1,
		files: {
		"1":{
				imgFile: ["../images/TVO/03-inform-zone5.png", "../images/POV/02-inform.png"],
				audFileIdx: [28, 28]
			},
		"2":{
				imgFile: ["../images/TVO/06-warning-zone5.png", "../images/POV/04-warning-road.png"],
				audFileIdx: [29, 29]
			}}},
	"33": {
		zoneName: "InRoadwayFwdCurbPed",
		timerId: null,
		currentAudIdx: -1,
		files: {
		"1":{
				imgFile: ["../images/TVO/07-warning-zone3.png", ""], // no inform image present
				audFileIdx: [29, -1] // using warning audio
			},
		"2":{
				imgFile: ["../images/TVO/07-warning-zone3.png", ""],
				audFileIdx: [29, -1]
			}}},
	"34": {
		zoneName: "InRoadwayRearCenterPed",
		timerId: null,
		currentAudIdx: -1,
		files: {
		"1":{
				imgFile: ["../images/TVO/04-inform-zone6.png", "../images/POV/03-warning.png"],
				audFileIdx: [28, 29]
			},
		"2":{
				imgFile: ["../images/TVO/09-warning-zone6.png", "../images/POV/03-warning.png"],
				audFileIdx: [29, 29]
			}}},
	"35": {
		zoneName: "InRoadwayRearCurbPed",
		timerId: null,
		currentAudIdx: -1,
		files: {
		"1":{
				imgFile: ["../images/TVO/08-warning-zone4.png", "../images/POV/03-warning.png"],  // no inform image present
				audFileIdx: [29, 29]// using warning audio
			},
		"2":{
				imgFile: ["../images/TVO/08-warning-zone4.png", "../images/POV/03-warning.png"],
				audFileIdx: [29, 29]
			}}}
};
function TspwDisplayAlert(json_obj) {
    switch (json_obj.EventCode) {
        case "3": // Entered Area
            TSPWActive = true;
            clearImage();
            populateHisImageMessageObject(json_obj.AppId, json_obj.Id, json_obj.Severity);
            TspwDisplayBase(json_obj);
            TspwClearAllZones();
            if (DebugOutput) console.log("Displayed TSPW");
            break;
        case "4": // Exited Area
            TSPWActive = false;
            populateHisImageMessageObject(json_obj.AppId, json_obj.Id, json_obj.Severity);
            TspwClearAllZones();
            clearImage();
            setImage(commonBackgroundImage, "");
            if (DebugOutput) console.log("Stopped displaying TSPW");
            displayNextAlert();
            break;
        case "5": // Detected Area
            // No Action
            break;
        case "20": // EventComplete
        case "21": // InArea
            // No pedestrian alerts while on the map.
            TspwClearAllZones();
            if (DebugOutput) console.log("Cleared All TSPW Zones");
            break;
        case "10": // OnCurbsidePed
        case "32": // InRoadwayFwdCenterPed
        case "33": // InRoadwayFwdCurbPed
        case "34": // InRoadwayRearCenterPed
        case "35": // InRoadwayRearCurbPed
            TspwDisplayZone(json_obj);
            break;
    }
}

function TspwHandleAlert(json_obj)
{
    switch (json_obj.EventCode) {
    case "3": // Entered Area
        if (DebugOutput) console.log("Entered TSPW");
        break;
    case "4": // Exited Area
        if (DebugOutput) console.log("Exited TSPW");
        break;
    case "5": // Detected Area
        // No Action
        break;
    case "20": // EventComplete
    case "21": // InArea
    	// No pedestrian alerts while on the map.
        if (DebugOutput) console.log("Cleared All TSPW Zones");
    	break;
    case "10": // OnCurbsidePed
    case "32": // InRoadwayFwdCenterPed
    case "33": // InRoadwayFwdCurbPed
    case "34": // InRoadwayRearCenterPed
    case "35": // InRoadwayRearCurbPed
    	break;
	}
}

function TspwDisplayBase(json_obj)
{
	var isPov = (json_obj.AppId == "13");
	
	if (isPov) {
        commonBackgroundImage = '../images/POV/01-base.png';
    	setImage(commonBackgroundImage, json_obj.CustomText);
    }
	else {
        commonBackgroundImage = '../images/TVO/02-base.png';
    	setImage(commonBackgroundImage, json_obj.CustomText);
    }
}

function TspwDisplayZone(json_obj)
{
	var evCode = json_obj.EventCode;
	var sev = json_obj.Severity;
	var zoneObj = tspwZoneInfoTable[evCode];
    var povIdx = (json_obj.AppId == "8")? 0 : 1;
//	var povIdx = json_obj.Id;
	
    if (sev in tspwValidSeverity) {
		var audFileIdx = zoneObj.files[sev].audFileIdx[povIdx];
		if (audFileIdx > TspwGetAudioSeverity()) {
			// Play new higher severity audio
			playAudio(audFileIdx, audioFiles);
		}
		zoneObj.currentAudIdx = audFileIdx;
		
		var zone = $("#" + zoneObj.zoneName + "ZoneImage");
		
		var imgFile = zoneObj.files[sev].imgFile[povIdx];

		// Make sure Base image is set
        TspwDisplayBase(json_obj);
		
		if (imgFile) {
		    zone.attr('src', imgFile);
            if (zone.hasClass("hidden")) {
                populateHisImageMessageObject(json_obj.AppId, json_obj.Id, json_obj.Severity);

                his.payload.HISPreState = commonBackgroundImage;
                his.payload.HISPostState = imgFile;
                sendCommand("TEST;" + JSON.stringify(his));   
                zone.removeClass("hidden");
            }
		    
		    // Reset zone display timer.
			clearTimeout(zoneObj.timerId);
			if (json_obj.DisplayDuration == null || json_obj.DisplayDuration == undefined || isNaN(json_obj.DisplayDuration) || json_obj.DisplayDuration == "" || json_obj.DisplayDuration == 0) {
			    zoneObj.timerId = setTimeout(TspwClearZone, tspwPedestrianTimout_ms, evCode, json_obj.AppId, json_obj.Id, json_obj.Severity);
			} else {
			    zoneObj.timerId = setTimeout(TspwClearZone, json_obj.DisplayDuration, evCode, json_obj.AppId, json_obj.Id, json_obj.Severity);
			}
		    if (DebugOutput) console.log("Set Image for : " + zoneObj.zoneName);
		}
		
	    // TODO: remove below - test only
//	    TspwDisplayAllZones("2");
   	}
	else {
	   if (DebugOutput)  console.log("Received invalid alert severity " + sev + " for TSPW display event: " + evCode);
	}
}

function TspwClearZone(evCode, appId, id, severity)
{
	var zoneObj = tspwZoneInfoTable[evCode];
    var zone = $("#" + zoneObj.zoneName + "ZoneImage");
    if (!zone.hasClass("hidden")) {
        populateHisImageMessageObject(appId, id, severity);

        his.payload.HISPreState = zone.attr("src");
        his.payload.HISPostState = commonBackgroundImage;
        sendCommand("TEST;" + JSON.stringify(his));   
        zone.addClass("hidden");
    }
    clearTimeout(zoneObj.timerId);
    zoneObj.timerId = null;
    zoneObj.currentAudIdx = -1;
}

function TspwClearAllZones()
{
    Object.keys(tspwZoneInfoTable).forEach(function (key, index) {
        // key: the name of the object key
        // index: the ordinal position of the key within the object
        TspwClearZone(key, "0");
    });
    $("#bottomDiv").html("");
}

function TspwGetAudioSeverity() {
	var audIdx = -1;
	Object.keys(tspwZoneInfoTable).forEach(function(evCode,index) {
		// Return highest severity audio that was played for any of the zones
		if (audIdx < tspwZoneInfoTable[evCode].currentAudIdx)
			audIdx = tspwZoneInfoTable[evCode].currentAudIdx;
	});
	return audIdx;
}

//TODO: remove below - test only
function TspwDisplayAllZones(sev)
{
	Object.keys(tspwZoneInfoTable).forEach(function(evCode,index) {
		var zoneObj = tspwZoneInfoTable[evCode];
		if (sev in tspwValidSeverity) {
			var zone = $("#" + zoneObj.zoneName + "ZoneImage");		
			zone.removeClass("hidden");
		    zone.attr('src', zoneObj.files[sev].imgFile);
		}
	});
}
//------- TSPW End -------------------------------------------------------


// Sounds -----------------------

var audioFiles = [
    "../sounds/Caution_Peddestrian_Ahead.mp3",      // 0
    "../sounds/Warning_Pedestrian_Ahead.mp3",
    "../sounds/Caution_Peddestrian_On_Right.mp3",
    "../sounds/Warning_Peddestrian_On_Right.mp3",
    "../sounds/Caution_Vehicle_Ahead-Audio.mp3",
    "../sounds/Warning_Vehicle_Ahead-Track 2.mp3",
    "../sounds/Caution_Vehicle_On_Left.mp3",
    "../sounds/Caution_Vehicle_Crossing_Path.mp3",
    "../sounds/FCW.wav",
    "../sounds/Train_Honk.wav",                     // 9
// -----------------------------------------------------    
    "../sounds/E-PCW/E-PCW_caution_far.mp3",        // 10
    "../sounds/E-PCW/E-PCW_caution_left.mp3",
    "../sounds/E-PCW/E-PCW_caution_near.mp3",
    "../sounds/E-PCW/E-PCW_caution_near_caution_far.mp3",
    "../sounds/E-PCW/E-PCW_caution_near_caution_left.mp3",
    "../sounds/E-PCW/E-PCW_caution_near_caution_right.mp3", //15
    "../sounds/E-PCW/E-PCW_caution_right.mp3",
    "../sounds/E-PCW/E-PCW_warning_far.mp3",
    "../sounds/E-PCW/E-PCW_warning_left.mp3",
    "../sounds/E-PCW/E-PCW_warning_near.mp3",

    "../sounds/E-PCW/E-PCW_warning_near_caution_far.mp3",       // 20
    "../sounds/E-PCW/E-PCW_warning_near_warning_left.mp3",
    "../sounds/E-PCW/E-PCW_warning_near_warning_right.mp3",
    "../sounds/E-PCW/E-PCW_warning_right.mp3",
    "../sounds/E-PCW/E-PCW_warning_near_warning_far.mp3",
    "../sounds/EEBL.wav",
    "../sounds/EEBL_warning.wav",
 // -----------------------------------------------------    
    "../sounds/TSPW/TSPW_Caution\ Curb.mp3",		// 27
    "../sounds/TSPW/TSPW_Caution\ in\ Road.mp3",           
    "../sounds/TSPW/TSPW_Warning\ in\ Road.mp3",
	"../sounds/RCVW/Inform\ Alert\ lowering_mixdown\ 200ms.wav", //30
	"../sounds/RCVW/Warn\ Prototype\ 3\ Long_mixdown.wav"
];


for (var i in audioFiles) {
    preloadAudio(audioFiles[i]);
}

$(function(){
    $("[data-role='header']").toolbar();
});

$(document).ready(function () {
    $(".headbtn").mousedown(function () {
        $(".headbtn").removeClass('ui-btn-active');
        $(this).addClass('ui-btn-active');
    });
});
