<?php

$sasKeyName = "CBSReceiver";
$sasKeyValue = "hdJ+udbmcmgoM9BZvGm/0P0yHpXKt/NZt+ASbAxAnwg=";

$targetUri = strtolower(rawurlencode("https://rcvwcbssb.servicebus.windows.net"));

$expires = time();
$expiresInMins = 60;
$week = 60 * 60 * 24 * 7;
$expires = $expires + $week; 
$toSign = $targetUri . "\n" . $expires; 

$signature = rawurlencode(base64_encode(hash_hmac('sha256', $toSign, $sasKeyValue, TRUE))); 

$token = "SharedAccessSignature sr=" . $targetUri . "&sig=" . $signature . "&se=" . $expires . "&skn=" . $sasKeyName; 

$headers = [
    "Authorization:" . $token,
    "Host: rcvwcbssb.servicebus.windows.net",
    "Content-Length: 0"];

?>