<?php
// send a video file to the client to be displayed

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

$file_path = query_video_path($conn, $video_id);
if (is_null($file_path)) {
    exit_script_on_failure("FILE_ERROR");
}

$file_name = basename($file_path);
$play_dir = SITE_ROOT . '/test/';
$play_file = $play_dir . $file_name;


if (!copy($file_path, $play_file)) {
    exit_script_on_failure("COPY_ERROR1");
}


if (!rename($play_file, $play_dir . 'currentVideo')) {
    exit_script_on_failure("COPY_ERROR2");
}

echo json_encode(array('success' => TRUE));

?>
