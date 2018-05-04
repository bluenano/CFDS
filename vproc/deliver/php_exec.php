<?php

//Could maybe utalize integer process return values,
//or last line of output to singal errors.
//
//Could make last line the resulting filename, if is convenient.

echo "I'm in.\n";

$videoid = "6"; //string
$videofile = "/tmp/testvid.mp4";

$packet = $videofile." ".$videoid;

echo exec("./vidproc.out ".$packet);//space between exe
echo "\n"

?>
