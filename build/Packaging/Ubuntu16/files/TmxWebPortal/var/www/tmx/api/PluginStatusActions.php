<?php

require('../dbconnectionPDO.php');
require('Authentication.php');

if($_GET["action"] == "list")
{
	$query = "SELECT `plugin`.`id`, `plugin`.`name`, `plugin`.`description`,`plugin`.`version`, `pluginStatus`.`value` as status, `installedPlugin`.`enabled` ";
	$from = "FROM `plugin` JOIN `pluginStatus` ON `pluginStatus`.`pluginId` = `plugin`.`id` AND `pluginStatus`.`key` = '' LEFT JOIN `installedPlugin` ON `installedPlugin`.`pluginId` = `plugin`.`id`";

	$query .= $from;

	$count = $DBH->query("SELECT COUNT(*) " . $from);

	$recordCount = $count->fetchColumn();

	if(array_key_exists('jtSorting', $_GET) == TRUE)
	{
		$orderby = $_GET["jtSorting"];
		$DBH->quote($orderby);
		$query .= " ORDER BY " . $orderby;
	}

	if(array_key_exists('jtStartIndex', $_GET) == TRUE)
	{
		$start = (int)$_GET['jtStartIndex'];
		$pagesize = (int)$_GET['jtPageSize'];
		$query .= " LIMIT " . $start .", " . $pagesize;
	}

	$STH = $DBH->query($query);

	$STH->execute();

	//Add all records to an array
	$rows = array();
	while($row = $STH->fetch(PDO::FETCH_ASSOC)) {
    	$rows[] = $row;
	}
	 
	//Return result to jTable
	$jTableResult = array();
	$jTableResult['Result'] = "OK";
	$jTableResult['TotalRecordCount'] = $recordCount;
	$jTableResult['Records'] = $rows;
	print json_encode($jTableResult);
}
?>


