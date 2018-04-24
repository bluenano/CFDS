<?php

// $argv[0] = name of script
// $argv[1] = user id
// argv[2] = title
// $argv[3] = initial path to video

// returns video_id or -1 on failure


include_once '../shared/database.php';
include_once '../shared/utilities.php';


if (count($argv) != 4) {
    echo -1;
}


date_default_timezone_set('America/Los_Angeles');

$user_id = $argv[1];
$title = $argv[2];
$video_path = $argv[3];


// video table: 
// (userid, title, uploaddate, numframes, framespersecond, width, height, videopath)
// ffmpeg + ffprobe can get numframes, framespersecond, width, height

$get_num_frames = "ffprobe -v error -count_frames -select_streams v:0 -show_entries stream=nb_read_frames -of default=nokey=1:noprint_wrappers=1 $video_path";
$num_frames = (int) shell_exec($get_num_frames);


$get_fps = "ffprobe -v error -select_streams v:0 -show_entries stream=avg_frame_rate -of default=noprint_wrappers=1:nokey=1 $video_path";
$fps = shell_exec($get_fps);
$fps = (int) parse_fps($fps);


$get_resolution = "ffprobe -v error -of flat=s=_ -select_streams v:0 -show_entries stream=height,width $video_path";
$resolution = shell_exec($get_resolution);
$resolution = parse_resolution($resolution);
$width = (int) $resolution[0];
$height = (int) $resolution[1];


$conn = connect();
if (is_null($conn)) {
    echo -1;
}

// get userid from SESSION or client
$upload_date = date('Y-m-d');
$video_id = insert_video($conn, $user_id, $title, $upload_date,
             $num_frames, $fps, $width, $height, 
             $video_path);

// $video_id may be null or false
echo (is_null($video_id) || !$video_id) ? -1 : $video_id; 

?>