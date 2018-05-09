<?php
// given a video id, remove the video id from the database
// and delete the file from the uploads directory
// usage: php delete.php video_id path_to_video

include_once '../../../config.php';
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
$path = query_video_path($conn, $video_id);


if (!remove_video($conn, $video_id)) {
    exit_script_on_failure("DELETE_ERROR");
}

$vdata_path = VDATA_DIR . 'vdata_' . $video_id . '.dat';
unlink($vdata_path);
unlink($path);

echo json_encode(array('success' => TRUE));

?>