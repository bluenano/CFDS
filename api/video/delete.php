<?php
// given a video id, remove the video id from the database
// and delete the file from the uploads directory
// usage: php delete.php video_id path_to_video

include_once '../shared/database.php';
include_once '../shared/utilities.php';

if (count($argv) != 2) {
    exit_script_on_failure("USAGE_ERROR");
}

// video id is passed from controller
$video_id = $argv[1];

$conn = connect();
if (is_null($conn)) {
    exit_script_on_failure("CONNECTION_ERROR");
}


// delete from db
if (!remove_video($conn, $video_id)) {
    exit_script_on_failure("DELETE_ERROR");
}

// delete from video directory
$path = query_video_path($conn, $video_id);
unlink($path);

echo json_encode(array('success' => TRUE));

?>