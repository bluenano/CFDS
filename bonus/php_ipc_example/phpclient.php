<?php

$me = "/tmp/unix_client";
$them = "/tmp/unix_server";

$socket = socket_create(AF_UNIX, SOCK_DGRAM, 0);

if (!socket_set_option($socket, SOL_SOCKET, SO_REUSEADDR, 1)) {
    echo 'Unable to set option on socket: '. socket_strerror(socket_last_error()) . PHP_EOL;
    exit(1);
}

unlink($me);
if (socket_bind($socket, $me) === false) {
    echo "self bind failed\n";
    exit(1);
}

if (socket_connect($socket, $them) === false) {
    echo "connect failed\n";
    exit(1);
}

$msg = "can you process this for me? 0.mp4\0";
if (socket_send($socket, $msg, strlen($msg), 0)===false) {
    echo "send failed\n";
    exit(1);
}

if (socket_recv($socket, $buf, 512, 0)===false) {
    echo "recv failed\n";
    exit(1);
}

echo "received: " . $buf . "\n";

?>
