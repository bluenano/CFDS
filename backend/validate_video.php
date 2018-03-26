<?php

// usage: php validate_video.php video_file.ext
// argv[1] = the video file to validate

echo "<pre>";
	print_r($_FILES);
	echo "</pre>";


if (count($argv) != 2) {
	echo 0;
	exit;
}

$cmd = "ffmpeg -v error -i $argv[1] -map 0:1 -f null - 2>error.log";
shell_exec($cmd);

$contents = file_get_contents('error.log');

// if the file is empty, then the video is valid
$result = strlen($contents) == 0 ? 1 : 0;

echo $result . "\n";

?>