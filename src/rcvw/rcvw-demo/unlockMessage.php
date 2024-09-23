<?php

require('sbSetup.php');

$url = 'https://rcvwcbssb.servicebus.windows.net/nate/subscriptions/nate/messages/' . $_GET['messageId'] . '/' . $_GET['messageToken'] . '?timeout=60';

$ch = curl_init($url);

curl_setopt($ch, CURLOPT_URL, $url);
curl_setopt($ch, CURLOPT_POST, 1);
curl_setopt($ch, CURLOPT_HEADER, 1);
curl_setopt($ch, CURLOPT_HTTPHEADER, $headers);
curl_setopt($ch, CURLOPT_RETURNTRANSFER, true);

$response = curl_exec($ch);

$info = curl_getinfo($ch);

if ( $info['http_code'] > 201 ) 
    throw new Exception('Error sending notification: '. $info['http_code'] . ' msg: ' . $response);

curl_close($ch);

echo $response;

?>