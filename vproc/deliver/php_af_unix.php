<?php

/* SAMPLE OUTPUT:

    FIRST IN SEPARATE TAB:

    jw@jw-laptop ~/CS160Project/bin $ ./vidproc.out 
    spawning daemon
    waiting for packet...
    
    # (then run this php)

    OpenCV: FFMPEG: tag 0x31637661/'avc1' is not supported with codec id 28 and format 'mp4 / MP4 (MPEG-4 Part 14)'
    OpenCV: FFMPEG: fallback to use tag 0x00000021/'!???'
    testvid.mp4
    fps according to opencv: 17
    entering read/process loop
    INSERT INTO frame(videoid, framenumber, ftpupilrightx, ftpupilrighty, ftpupilleftx, ftpupillefty, roll, pitch, yaw)VALUES (6, 0, -1, -1, 408, 203, -0.073931, 0.433284, -3.058731) RETURNING frameid
    INSERT INTO openfacedata(pointnumber, x, y, frameid) VALUES (1, 364, 213, 1744)
    UPDATE video SET (numframes, framespersecond, width, height) = (40, 17, 640, 480) WHERE videoid = 6
    Everything except most initialization took about 1253 ms
    sendto(): Success
    tt testvid.mp4waiting for packet...


THIS TAB:

    jw@jw-laptop ~/CS160Project/vproc/deliver $ php -f php_af_unix.php 
    output video file relative to (dir of this php file?): [testvid.mp4]
*/


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
$videofile = "/usr/local/share/testvid.mp4";

$packet = $videofile . ' ' . $videoid;

if (socket_sendto($socket, $packet, strlen($packet), 0, "/tmp/myserver.sock", 0) === false) {
    perror("sendto");
}

if (socket_recvfrom($socket, $sres, 512, 0, $source) === false) {
    perror("recv_from");
}

socket_close($socket);
unlink($file);

if (strlen($sres) < 3) {
    echo "error occured\n";
    exit(-1);    
}

if ($sres[0]!=='t') {
    echo "no video reported to be made\n";
}

if ($sres[1]!=='t') {
    echo "no transaction committed to databse reported\n";
}

echo "output video file relative to (dir of this php file?): [".substr($sres, 3)."]\n";

?>
