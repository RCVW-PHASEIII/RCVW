// Mobile app to full screen
window.addEventListener("load",function() {
	// Set a timeout...
	setTimeout(function(){
		// Hide the address bar!
		window.scrollTo(0, 1);
	}, 0);
});
var bPopup = false;
var popupQueue = [];
var popupOption = "beeps";
var soundActive = false;
var volume = 1;
var setPersistentCommonValuesFlag = false;
var replaceCommonNamesFlag = false;
var iframes = [];
var ackMessage = "";

var bAddedExcom = false;

var width = Math.floor(100 / topNav.length) - 0.5;

function navTab() {
    var output = "<div data-role='header' data-position='fixed' class='headercontainer' data-tap-toggle='false' data-theme='a' data-fullscreen='true'> ";
    output += "<div class='buttoncontainer margin-left-0em' data-position='fixed'>";
    for (var i = 0; i < topNav.length; i++) {
	    output += "<button id='btn_" + topNav[i].name + "' class='mytab ui-btn ui-btn-inline ui-corner-all' style='width:" + width + "% !important;'  ondblclick=\"reloadPage(" + topNav[i].name +  ")\" onmousedown=\"location.href='#" + topNav[i].location + "'\">" + topNav[i].name + "</button>";
    }
	output += "</div></div>";
	document.getElementById("navbar").innerHTML = output;
}

function navDiv(){
	var wrapper = document.getElementById("tabs");

	for (var i = 0; i < topNav.length; i++) {
	    var link = topNav[i].lnkname;
	    if (topNav[i].lnkname.indexOf("..") != -1) {
	        link = topNav[i].lnkname.replace("..", "");
	        link = window.document.baseURI.replace("/main/main.html", link);
	    }
	    wrapper.innerHTML += "<div id='" + topNav[i].name + "' class='ui-body-a ui-content uipage' data-state='inactive'><div role='main' class='ui-content iframecontainer'><iframe src='" + topNav[i].lnkname + "' frameBorder='0' id='iframe_" + topNav[i].name + "' ></iframe></div></div>";
	    iframes.push({ location: topNav[i].name, target: window.frames[i], link: link, hasBeenAcked: false });
    }
}

navTab();
navDiv();

function switchPopupOption(newOption) {
    popupOption = newOption;
}


function timeStamp(time)
{
    if (time == null)
    time = new Date();
    var timeString = "";
    
    var hours = time.getHours().toString();
    if (hours.length == 1) {
        hours = "0" + hours;
    }

    var minutes = time.getMinutes().toString();
    if (minutes.length == 1) {
        minutes = "0" + minutes;
    }

    var seconds = time.getSeconds().toString();
    if (seconds.length == 1) {
        seconds = "0" + seconds;
    }

    var timeString = hours + ":" + minutes + ":" + seconds;// + "." + time.getMilliseconds();
    return timeString;
}

function createPopupWindow() {
    var output = "<a href='#popupalert' data-rel='popup' id='popupwindow' class='hidden'>Show Popup</a>";
    output += "<div data-role='popup' id='popupalert' data-dismissible='false' class='opverifypopup hidden' data-position-to='window' data-state='active'>";
    output += "<h2 id='popupdevice' class='center'></h2><h1 id='popuptype' data-msgNum='' class='center'></h1><div id='popupmsg' class='popupmsgbox block'></div>";
    output += "<a href='#' id='popupACK' class='ui-btn ui-corner-all ui-shadow ui-btn-none confirmbutton'>OK</a>";
    output += "<a href='#' id='popupCancel' class='ui-btn ui-corner-all ui-shadow ui-btn-none confirmbutton hidden'>Cancel</a>";
    output += "</div></div>";
    document.getElementById("popup").innerHTML = output;
}

function generateAckMessageForPopup() {
    var device = window.document.getElementById('popupdevice').innerHTML;
    var type = window.document.getElementById('popuptype').innerHTML;
    var string = window.document.getElementById('popupmsg').innerHTML;
    var msgNum = window.document.getElementById('popuptype').getAttribute('data-msgNum');

    var ackMessage = device;
    var pageDevice = [];

    pageDevice.push(device);

    if (type == "DANGER" || type == "WARNING") {
        var alertType = "";
        if (type == "DANGER") alertType = "RED_LIMIT";
        else if (type == "WARNING") alertType = "YELLOW_LIMIT";
        if (string.indexOf("Lower") != -1) alertType += "_LOW";
        else if (string.indexOf("Upper") != -1) alertType += "_HIGH";
        var sensor = string.substring(0, string.indexOf(":"));
        ackMessage += ";" + 0 + ";LIMIT;" + sensor;
        ackMessage += ";" + alertType + ";ACKNOWLEDGE";
        console.log("ACK:" + ackMessage + " PageDevice:" + pageDevice);
    } 
    for (var j = 0; j < pageDevice.length; j++) {
        for (var i = 0; i < iframes.length; i++) {
            if ((iframes[i].location != undefined && iframes[i].location.toUpperCase().indexOf(pageDevice[j].toUpperCase()) != -1) ||
             (iframes[i].location != undefined && pageDevice[j].toUpperCase().indexOf(iframes[i].location.toUpperCase()) != -1) && iframes[i].hasBeenAcked) {
                window[i].postMessage("sendMsg@$" + ackMessage, iframes[i].link);
                break;
            }
        }
    }
}

function reloadPage(frame)
{
    console.log(frame);
    console.log("Reloading Page " + frame.id);
    document.getElementById("iframe_" + frame.id).src = document.getElementById("iframe_" + frame.id).src;
    for (var i = 0; i < iframes.length; i++) {
        if (iframes[i].location.indexOf(frame.id) != -1) {
            iframes[i].hasBeenAcked = false;
        }
    }
}

function setPersistentValues() {
    for (var i = 0; i < persistentMemory.length; i++) {
        window[persistentMemory[i].tag] = persistentMemory[i].value;
    }
    setPersistentValuesFlag = true;
}

function popupAck() {
    bPopup = false;
    generateAckMessageForPopup();
    if (popupOption == 'beeps') {
        stopWave();
    } else if (popupOption == 'voice') {
        cancelSpeech();
    }
    closePopup();
    showPopupMessage();
}

function updatePopupOption() {
    if (window.popupOption != popupOption) {
        switchPopupOption(window.popupOption);
    }
}

function showPopupMessage() {
    if (popupQueue.length <= 0) {
        return;
     }
    var x = popupQueue.shift();
    var msg = x.msg;
    var tokens = msg.split(";");
    if (tokens.length >= 4 && !(tokens[0].length <= 0 || tokens[2].length <= 0)) {
        if (tokens[2].toUpperCase() == "EVENT" || tokens[2].toUpperCase() == "LIMIT") {
            showAlertMessage(msg);
        } 
    }
    else {
        showPopupMessage();
    }
} 

window.AudioContext = window.AudioContext || window.webkitAudioContext;
var context = new window.AudioContext();

var currentOsc, gainOsc, speech;
var audioTimer, soundActive = false, overallAudioCounter = 0;


function playBeeps(freq, timeDuration, numberOfBeeps, continuousFlag) {
    createWave(freq);
    currentOsc.start();
    audioTimer = setInterval(function () {
        if (window.overallAudioCounter < numberOfBeeps) {
            window.soundActive = true;
            if (!continuousFlag) {
                if (window.overallAudioCounter % 2 == 0) {
                    window.gainOsc.gain.value = -1;
                } else {
                    window.gainOsc.gain.value = beepVolumes();
                }
            } else {
                window.gainOsc.gain.value = beepVolumes();
            }
            window.overallAudioCounter++;
        } else {
            stopWave();
        }
    }, timeDuration);
}

function adjustVolume(val) {
    window.volume = val / 100.0;
    if (gainOsc != null && gainOsc != undefined) {
        window.gainOsc.gain.value = window.volume;
    }
}

function movePopups(direction) {
    if (direction == "back") {
        window.document.getElementById("popupalert").style.zIndex = -1;
    } else {
        window.document.getElementById("popupalert").style.zIndex = 16002;
    }
}

function printPopupQueue()
{
    console.log("PopupQueue Start -----------------------------------------------");
     for (var i = 0; i < popupQueue.length; i++) {
        console.log(popupQueue[i].msg);
     }
    console.log("PopupQueue End -------------------------------------------------");
}

// Take touch events and convert them to mouse events so the events
// occur in the same sequence between mouse and touch
function touchHandler(event)
{
    var touches = event.changedTouches, first = touches[0], type = "";
    switch(event.type)
    {
        case "touchstart": type = "mousedown"; break;
        case "touchmove":  type = "mousemove"; break;        
        case "touchend":   type = "mouseup"; break;
        default: return;
    }
    var simulatedEvent = document.createEvent("MouseEvent");
    simulatedEvent.initMouseEvent(type, true, true, window, 1, first.screenX, first.screenY, first.clientX, first.clientY, 
                                false, false, false, false, 0/*left*/, null);
    first.target.dispatchEvent(simulatedEvent);
    event.preventDefault();
}


var prevNavBtn = null, prevHeadBtn = null;


$(document).ready(function () {
    setPersistentCommonValues();
    setPersistentValues();

/*
    $(document).on("mousedown touchstart",function(e){
//         console.log(e);
//         $(window).trigger('resize');
//       $(document).hide().show(0);
    });

    // Send touch events to touchHandler to convert to mouse events
    document.addEventListener("touchstart", touchHandler, true);
    document.addEventListener("touchmove", touchHandler, true);
    document.addEventListener("touchend", touchHandler, true);
    document.addEventListener("touchcancel", touchHandler, true);    
*/
    $(".mytab").click(function () {
        $(".mytab").removeClass('ui-btn-active');
        $(this).addClass('ui-btn-active');
        $(".uipage").attr("data-state", "inactive");
        var pageName = $(this).html();
        var page = document.getElementById(pageName);
        if (page.getAttribute("data-state") == "inactive") {
            page.setAttribute("data-state", "active");
        }
    });
    createPopupWindow();

    if (popupOption == 'voice') {
        $('#popupSound').addClass('ui-btn-active');
    } else {
        $('#popupBeeps').addClass('ui-btn-active');
    }

    document.getElementsByClassName('mytab')[0].click();

    window.document.getElementById("popupACK").onclick = function (event) {
        popupAck();
        event.cancelBubble = true;
        event.stopImmediatePropagation();
    }

    window.addEventListener("message", function (event) {
        var tokenPairs = event.data.split("@#");
        for (var i = 0; i < tokenPairs.length; i++) {
            var tokens = tokenPairs[i].split("@$");
            switch (tokens[0]) {
                case "adjustVolume":
                    adjustVolume(tokens[1]);
                    break;
            }
        }
    });
});
