<?php

$db = new PDO("sqlsrv:Server=rcvw-db-server.database.windows.net;Database=rcvw-db", "dba", "B@ttelle");

$crossingID = $_GET["crossingID"];
$status = $_GET["status"];
$crossingField = $_GET["field"];

$sql = "UPDATE [FL_LocationAndClassification] SET [Manual" . $crossingField . "] = '" . $status . "' WHERE [CrossingID] = '" . $crossingID . "'";

$db -> exec($sql);

echo $sql;

?>