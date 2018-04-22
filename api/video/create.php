<?php

// $argv[0] = name of script
// $argv[1] = user id
// argv[2] = title
// $argv[3] = path to video
 
include_once '../shared/database.php';
include_once '../shared/utilities.php';


if (count($argv) != 4) {
    // invalid usage
    echo "Program usage: create.php userid path";
}


date_default_timezone_set('America/Los_Angeles');

$id = $argv[1];
$title = $argv[2];
$videopath = $argv[3];

// use shell_exec() or exec()?
// exec gives the program's exit code

// video table: 
// (userid, title, uploaddate, numframes, framespersecond, width, height, videopath)
// ffmpeg + ffprobe can get numframes, framespersecond, width, height

$get_numframes = "ffprobe -v error -count_frames -select_streams v:0 -show_entries stream=nb_read_frames -of default=nokey=1:noprint_wrappers=1 $videopath";
$num_frames = (int) shell_exec($get_numframes);


$get_fps = "ffprobe -v error -select_streams v:0 -show_entries stream=avg_frame_rate -of default=noprint_wrappers=1:nokey=1 $videopath";
$fps = shell_exec($get_fps);
$fps = (int) parse_fps($fps);


// get width and height
$get_resolution = "ffprobe -v error -of flat=s=_ -select_streams v:0 -show_entries stream=height,width $videopath";
$resolution = shell_exec($get_resolution);
$resolution = parse_resolution($resolution);
$width = (int) $resolution[0];
$height = (int) $resolution[1];


$conn = connect();
if (is_null($conn)) {
    // db connection failed
    // do something here
}

// get userid from SESSION or client
$userid = 1;
$upload_date = date('Y-m-d');
insert_video($conn, $userid, $title, $upload_date,
             $num_frames, $fps, $width, $height, 
             $videopath);
?>