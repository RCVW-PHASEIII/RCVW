<?php

$db = new PDO("sqlsrv:Server=rcvw-db-server.database.windows.net;Database=rcvw-db", "dba", "B@ttelle");

$answerText = "{";

$row = $db -> query("SELECT * FROM [HRI_CROSSINGS] WHERE [CrossingID] = '$_GET[crossingID]'") -> fetch();
$manualRow = $db -> query("SELECT * FROM [FL_LocationAndClassification] WHERE [CrossingID] = '$_GET[crossingID]'") -> fetch();

$answerText .= '"CrossingID": "' .$row["CrossingID"] . '",';
$answerText .= '"CityName": "' . $row["CityName"] . '",';
$answerText .= '"CountyName": "' . $row["CountyName"] . '",';
$answerText .= '"Street": "' . $row["Street"] . '",';
$answerText .= '"Highway": "' . $row["Highway"] . '",';
$answerText .= '"RailroadCode": "' . $row["RailroadCode"] . '",';

$isInstrumented = ! is_null($row['RBSOperational']);

if ( is_null($row['RBSOperational']) ) {
    $answerText .= '"RBSOperational": "unknown", "RBSOperationalCode": -1,';
    $answerText .= '"PreemptionStatus": "unknown", "PreemptionStatusCode": -1,';
} else {
    if ( '0' == $row['RBSOperational'] ) {
        $answerText .= '"RBSOperational": "Not Operational", "RBSOperationalCode": 0,';
        $answerText .= '"PreemptionStatus": "unknown", "PreemptionStatusCode": -1,';
    } else {
        $answerText .= '"RBSOperational": "Operational", "RBSOperationalCode": 1,';
        if ( '0' == $row['PreemptionStatus'] )
            $answerText .= '"PreemptionStatus": "Not Active", "PreemptionStatusCode": 0,';
        else
            $answerText .= '"PreemptionStatus": "Active", "PreemptionStatusCode": 1,';
    }

}

if ( is_null($manualRow["ManualRBSOperational"]) ) {
    $answerText .= '"ManualPreemptionStatus": "unknown", "ManualPreemptionStatusCode": -1,';
    $answerText .= '"ManualRBSOperational": "unknown", "ManualRBSOperationalCode": -1,';
} else {

    if ( '0' == $manualRow['ManualRBSOperational'] ) {
        $answerText .= '"ManualRBSOperational": "Not Operational", "ManualRBSOperationalCode": 0,';
        $answerText .= '"ManualPreemptionStatus": "unknown", "ManualPreemptionStatusCode": -1,';
    } else {
        $answerText .= '"ManualRBSOperational": "Operational", "ManualRBSOperationalCode": 1,';
        if ( '0' == $manualRow['ManualPreemptionStatus'] )
            $answerText .= '"ManualPreemptionStatus": "Not Active", "ManualPreemptionStatusCode": 0,';
        else
            $answerText .= '"ManualPreemptionStatus": "Active", "ManualPreemptionStatusCode": 1,';
    }
}

$answerText .= '"PolCont": "' .$row["PolCont"] . '",';
$answerText .= '"RrCont": "' . $row["RrCont"] . '",';
$answerText .= '"Latitude": "' . number_format($row["Latitude"],4) . '",';
$answerText .= '"Longitude": "' . number_format($row["Longitude"],4) . '",';
$answerText .= '"LrsMilePost": "' . $row["LrsMilePost"] . '",';
$answerText .= '"HwynDist": "' . $row["HwynDist"] . '",';
$answerText .= '"HwySpeed": "' . $row["HwySpeed"] . '",';

$crossingAngle = $row["XAngle"];

$angleText = "";

if ( "1" == $crossingAngle )
    $angleText = "0 - 29 deg";
else if ( "2" == $crossingAngle )
    $angleText = "30 - 59 deg";
else if ( "3" == $crossingAngle )
    $angleText = "69 - 90 deg";
else 
    $angleText = "unknown";

$answerText .= '"CrossingAngle": "' . $angleText . '",';

$crossingPosition = $row["PosXing"];

$anglePos = "";

if ( "1" == $crossingPosition )
    $anglePos = "At Grade";
else if ( "2" == $crossingPosition )
    $anglePos = "Under Railroad";
else if ( "3" == $crossingPosition )
    $anglePos = "Over Railroad";
else 
    $anglePos = "unknown";

$answerText .= '"CrossingPosition": "' . $anglePos . '",';

$flashText = "";
if ( '' == $row["FlashOth"] || empty($row["FlashOth"]) )
    $flashText = "None";
else if ( 0 < $row["FlashOth"] )
   $flashText = $row["FlashOth"];

$answerText .= '"GateConf": "' . ('' == $row["GateConf"] ? "No" : "Yes") . '",';
$answerText .= '"FlashPost": "' .('' == $row["FlashPost"] ? "No" : "Yes") . '",';
$answerText .= '"FlashOth": "' . $flashText . '",';

$answerText .= '"ErrorCode": ' . (is_null($row["ErrorCode"]) ? "0" : $row["ErrorCode"]) . ',';
$answerText .= '"ErrorMessage": "' . $row["ErrorMessage"] . '",';

$answerText .= '"IsInstrumented": ' . ( $isInstrumented ? "1" : "0" ) . '';

echo $answerText . '}';

?>