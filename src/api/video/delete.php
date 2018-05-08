<?php
// given a video id, remove the video id from the database
// and delete the file from the uploads directory
// usage: php delete.php video_id path_to_video

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

$frame_ids = query_frame_ids($conn, $video_id);
for ($i = 0; $i < count($frame_ids); $i++) {
    $frame_id = $frame_ids[$i]['frameid'];
    if (!remove_openface_data($conn, $frame_id)
        ||
        !remove_frame($conn, $frame_id)) {
        exit_script_on_failure("DATABASE_ERROR");
    }
}


if (!remove_video($conn, $video_id)) {
    exit_script_on_failure("DELETE_ERROR");
}

unlink($path);

echo json_encode(array('success' => TRUE));

?>