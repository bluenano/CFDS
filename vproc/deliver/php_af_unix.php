<?php

//adapted from https://stackoverflow.com/questions/1746207/how-to-ipc-between-php-clients-and-a-c-daemon-server/43422049#43422049

function perror($m) {
    echo $m.": ".posix_strerror(posix_get_last_error())."\n";
}

do {
  $file = sys_get_temp_dir() . '/' . uniqid('client', true) . '.sock';
} while (file_exists($file));

$socket = socket_create(AF_UNIX, SOCK_DGRAM, 0);//DGRAM's respect packet boundaries

if (socket_bind($socket, $file) === false) {
    perror("bind");
}

$videoid = "6"; //string
$videofile = "/tmp/testvid.mp4";

$packet = $videofile . ' ' . $videoid;

if (socket_sendto($socket, $packet, strlen($packet), 0, "/tmp/myserver.sock", 0) === false) {
    perror("sendto");
}

if (socket_recvfrom($socket, $buf, 512, 0, $source) === false) {
    perror("recv_from");
}


echo "received: [" . $buf . "]   from: [" . $source . "]\n";

socket_close($socket);
unlink($file);

?>
