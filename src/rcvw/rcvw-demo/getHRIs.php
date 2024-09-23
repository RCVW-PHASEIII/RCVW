<?php

$db = new PDO("sqlsrv:Server=rcvw-db-server.database.windows.net;Database=rcvw-db", "dba", "B@ttelle");

$validHRIs = array("FirstEntry","621223F","621221S","271824W","621275X","0000000","621220K","621219R","621217C",
                    "621216V","621211l","620727C","621209K","621208D","621207W","621206P","271823P",
                    "271820U","271819A","2711818T","271817L","271816E","271809U","271807F","271801P",
                    "271800H","621193R","271825D","271827S","271828Y","271829F");

if ( ! empty($_GET["crossingId"] ) ) {

    $specificId = $_GET["crossingId"];
    $sql = "SELECT tbl1.CrossingID,CAST(tbl1.Longitude as REAL) as lng,CAST(tbl1.Latitude as REAL) as lat, " .
            "PreemptionStatus,RBSOperational,ManualPreemptionStatus,ManualRBSOperational " .
            "FROM FL_LocationAndClassification tbl1,HRI_CROSSINGS tbl2 WHERE " .
                "tbl1.CrossingID = tbl2.CrossingID AND " .
                "tbl1.longitude IS NOT NULL AND tbl1.Latitude IS NOT NULL AND tbl1.CrossingID = '" . $specificId . "'";

} else {

    $neLng = $_GET["neLng"];
    $neLat = $_GET["neLat"];
    $swLng = $_GET["swLng"];
    $swLat = $_GET["swLat"];

    $sql = "SELECT tbl1.CrossingID,CAST(tbl1.Longitude as REAL) as lng,CAST(tbl1.Latitude as REAL) as lat, " .
                "PreemptionStatus,RBSOperational,ManualPreemptionStatus,ManualRBSOperational " .
                "FROM FL_LocationAndClassification tbl1,HRI_CROSSINGS tbl2 WHERE " .
                    "tbl1.CrossingID = tbl2.CrossingID AND " .
                    "tbl1.longitude IS NOT NULL AND tbl1.Latitude IS NOT NULL " .
                    "AND CAST(tbl1.Longitude as REAL) <= $neLng AND CAST(tbl1.Longitude as REAL) >= $swLng AND " .
                    "CAST(tbl1.Latitude as REAL) >= $swLat AND CAST(tbl1.Latitude as REAL) <= $neLat";

}

$answerText = "[";

$crossingIDs = array();

foreach ( $db -> query($sql,PDO::FETCH_NAMED) as $row ) {

    if ( ! array_search($row["CrossingID"],$validHRIs) )
        continue;

    $answerText .= '{';
    $answerText .= '"CrossingID": "' . $row["CrossingID"] . '",';
    $answerText .= '"Longitude": "' . $row["lng"] . '",';
    $answerText .= '"Latitude": "' . $row["lat"] . '",';

    if ( is_null($row['RBSOperational']) ) {
        if ( "0" == $row["ManualRBSOperational"] )
            $answerText .= '"PreemptionStatus": -1,';
        else
            $answerText .= '"PreemptionStatus": ' . (is_null($row["ManualPreemptionStatus"]) ? -1 : $row["ManualPreemptionStatus"]) . ',';
        $answerText .= '"RBSOperational": ' . (is_null($row["ManualRBSOperational"]) ? -1 : $row["ManualRBSOperational"]) . ',';
        $answerText .= '"IsInstrumented": 0,';
    } else {
        if ( "0" == $row["RBSOperational"] )
            $answerText .= '"PreemptionStatus": -1,';
        else
            $answerText .= '"PreemptionStatus": ' . $row["PreemptionStatus"] . ',';
        $answerText .= '"RBSOperational": ' . $row["RBSOperational"] . ',';
        $answerText .= '"IsInstrumented": 1,';
    }

    $answerText = substr($answerText,0,strlen($answerText) - 1) . '},';

    $crossingIDs[] = $row["CrossingID"];
}

echo substr($answerText,0,strlen($answerText) - 1) . "]";

/*

$answerText = "[";

$crossingIDs = array();

foreach ( $db -> query($sql,PDO::FETCH_NAMED) as $row ) {
    if ( ! array_search($row["CrossingID"],$validHRIs) )
        continue;
    $answerText .= '[["CrossingID","' . $row["CrossingID"] . '"],';
    $answerText .= '["Longitude","' . $row["lng"] . '"],';
    $answerText .= '["Latitude","' . $row["lat"] . '"],';

    if ( is_null($row['RBSOperational']) ) {
        if ( "0" == $row["ManualRBSOperational"] )
            $answerText .= '["PreemptionStatus",-1],';
        else
            $answerText .= '["PreemptionStatus",' . (is_null($row["ManualPreemptionStatus"]) ? -1 : $row["ManualPreemptionStatus"]) . '],';
        $answerText .= '["RBSOperational",' . (is_null($row["ManualRBSOperational"]) ? -1 : $row["ManualRBSOperational"]) . '],';
        $answerText .= '["IsInstrumented",0]],';
    } else {
        if ( "0" == $row["RBSOperational"] )
            $answerText .= '["PreemptionStatus",-1],';
        else
            $answerText .= '["PreemptionStatus",' . $row["PreemptionStatus"] . '],';
        $answerText .= '["RBSOperational",' . $row["RBSOperational"] . '],';
        $answerText .= '["IsInstrumented",1]],';
    }

    $crossingIDs[] = $row["CrossingID"];
}

echo substr($answerText,0,strlen($answerText) - 1) . "]";
*/

?>