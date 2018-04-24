<?php

// security checks
// check that the file is not empty
// check that file name is in english characters, numbers, and _-. symbols
// check that the file name is not larger than 250 characters
// check that the extension is supported

include_once '../../../config.php';
include_once '../shared/database.php';
include_once '../shared/utilities.php';


if ($_FILES[0]['error'] != UPLOAD_ERR_OK) {
    exit_script_on_failure('UPLOAD_ERROR');
}


if ($_FILES[0]['size'] == 0) {
    exit_script_on_failure('SIZE_ERROR');
}


if (!check_file_name_length($_FILES[0]['name'])
    ||
    !check_file_name($_FILES[0]['name'])) {
    exit_script_on_failure('NAME_ERROR');
}


$ext = end((explode('.', $_FILES[0]['name'])));
if (!check_extension(strtolower($ext))) {
    exit_script_on_failure('EXTENSION_ERROR');
}


$file_name = basename($_FILES[0]['name']);
$uploads_file = UPLOADS_DIR . $file_name;
$tmp_name = $_FILES[0]['tmp_name'];

if (!move_uploaded_file($tmp_name, $uploads_file)) {
    exit_script_on_failure('UPLOAD_ERROR');
} 


// use ffmpeg to validate the integrity of the video
if (!validate($uploads_file)) {
    unlink('error.log');
    exit_script_on_failure("VIDEO_ERROR");
} 
unlink('error.log');


$process_video = "./process_video.out $uploads_file";
exec($process_video);
// add error handling by checking exit code?


$out_file = 'out_' . $file_name;
if (!file_exists($out_file)) {
    exit_script_on_failure("PROCESSING_ERROR");
}


// get the user id from session or client http request
$user_id = 1;
$output = array();
$insert_into_db = "php ../video/create.php $user_id $file_name $out_file";
exec($insert_into_db, $output);


$video_id = (int) $output[0];
if ($video_id == -1) {
    exit_script_on_failure("INSERTDB_ERROR");
}


$final_location = UPLOADS_DIR . $video_id . '.' . $ext;
if (!rename($out_file, $final_location)) {
    exit_script_on_failure("MOVE_ERROR");
}

$conn = connect();
if (is_null($conn)) {
    exit_script_on_failure("CONNECTION_ERROR");
}

if (!update_video_path($conn, $video_id, $final_location)) {
    exit_script_on_failure("UPDATE_ERROR");
}

unlink($uploads_file);

echo json_encode(array('success' => TRUE));


?>