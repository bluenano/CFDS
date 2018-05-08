<?php
// send a video file to the client to be displayed

include_once '../shared/database.php';
include_once '../shared/utilities.php';


if (!isset($_GET['videoid'])) {
    exit_script_on_failure('GET_FAILURE');
}

$video_id = $_GET['videoid'];


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
