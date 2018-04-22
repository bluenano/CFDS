<?php
header('Content-Description: File Transfer');
header('Content-Type: application/octet-stream');
header('Content-Disposition: attachment; filename="'.basename($file).'"');
header('Expires: 0');
header('Cache-Control: must-revalidate');
header('Pragma: public');

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


if (!check_filename_length($_FILES[0]['name'])
    ||
    !check_filename($_FILES[0]['name'])) {
    exit_script_on_failure('NAME_ERROR');
}


$ext = end((explode('.', $_FILES[0]['name'])));
if (!check_extension(strtolower($ext))) {
    exit_script_on_failure('EXTENSION_ERROR');
}

$filename = basename($_FILES[0]['name']);
$uploads_file = UPLOADS_DIR . $filename;
$tmp_name = $_FILES[0]['tmp_name'];


if (!move_uploaded_file($tmp_name, $uploads_file)) {
    exit_script_on_failure('UPLOAD_ERROR');
} 

// now check that the video is undamaged
if (!validate($uploads_file)) {
    unlink('error.log');
    exit_script_on_failure("VIDEO_ERROR");
} 
unlink('error.log');


// now process the video
$process_video = "./process_video.out $uploads_file";
exec($process_video);
// add error handling by checking exit code?



// return video to client, might not need to do this
// we might return video to client when the user
// clicks on the Play button
$out_file = 'out_' . $filename;
if (file_exists($out_file)) {
    //readfile($out_file);
} else {
    exit_script_on_failure("PROCESSING_ERROR");
}


// now move the output video to a directory containing
// a user's videos 
// a user's directory will be UPLOADS_DIR/userid/
// now insert new video into database
// we can change this to be a flat file system later
$user_id = 1;
$user_dir = UPLOADS_DIR . $user_id . '/';
if (!file_exists($user_dir)) {
    mkdir($user_dir, 0775, true);
}

$path = $user_dir . $out_file;
rename($out_file, $path);

$insert_into_db = "php ../video/create.php $user_id $filename $path";
exec($insert_into_db);


echo json_encode(array('success' => TRUE));


?>