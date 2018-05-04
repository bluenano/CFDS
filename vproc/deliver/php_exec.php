<?php

/* SAMPLE OUTPUT:

jw@jw-laptop ~/CS160Project/vproc/deliver $ php -f php_exec.php 
I'm in.
OpenCV: FFMPEG: tag 0x31637661/'avc1' is not supported with codec id 28 and format 'mp4 / MP4 (MPEG-4 Part 14)'
OpenCV: FFMPEG: fallback to use tag 0x00000021/'!???'
testvid.mp4
entering read/process loop
output video file relative to (dir of this php file?): [testvid.mp4]
jw@jw-laptop ~/CS160Project/vproc/deliver $ 

*/

echo "I'm in.\n";

$videoid = "6"; //string
$videofile = "/usr/local/share/testvid.mp4";

$packet = $videofile." ".$videoid;

$sres = exec("../../bin/vidproc.out ".$packet);//space between exe

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
