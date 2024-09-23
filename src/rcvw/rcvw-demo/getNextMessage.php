<?php

require('./sbSetup.php');

$url = 'https://rcvwcbssb.servicebus.windows.net/nate/subscriptions/nate/messages/head?timeout=1';

/*
$headers = [
    "Authorization:" . $token,
    "Host: rcvwcbssb.servicebus.windows.net",
    "Content-Length: 0",
    'BrokerProperties: {"SequenceNumber: 3"}'];
*/

$ch = curl_init($url);

curl_setopt($ch, CURLOPT_URL, $url);
curl_setopt($ch, CURLOPT_CUSTOMREQUEST, "DELETE");
//curl_setopt($ch, CURLOPT_POST, 1);
curl_setopt($ch, CURLOPT_HEADER, 1);
curl_setopt($ch, CURLOPT_HTTPHEADER, $headers);
curl_setopt($ch, CURLOPT_RETURNTRANSFER, true);

$response = curl_exec($ch);

$info = curl_getinfo($ch);

curl_close($ch);

if ( $info['http_code'] > 299 ) 
    echo "Error:" . $info['http_code'] . " msg: " . $response;
else
    echo $response;

?>