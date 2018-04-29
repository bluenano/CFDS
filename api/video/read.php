<?php
// send a video file to the client to be displayed

include_once '../shared/database.php';
include_once '../shared/utilities.php';


if (count($argv) != 2) {
    exit_script_on_failure("USAGE_ERROR");
}

$video_id = $argv[1];

$conn = connect();
if (is_null($conn)) {
    exit_script_on_failure("CONNECTION_ERROR");
}

$file = query_video_path($conn, $video_id);
if (is_null($file)) {
    exit_script_on_failure("FILE_ERROR");
}


$path_info = pathinfo($file);
$ext = $path_info['extension'];

header("Content-Type: video/$ext");
header("Content-Length: " . filesize($file));
// using the path, send the file to the frontend

if (!readfile($file)) {
    exit_script_on_failure("TRANSFER_ERROR");
}    

?>
