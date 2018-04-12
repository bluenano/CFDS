<?php

// security checks
// check that the file is not empty
// check that file name is in english characters, numbers, and _-. symbols
// check that the file name is not larger than 250 characters
// check that the extension is supported

include_once '../config/database.php';
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


$uploads_dir = SITE_ROOT . '/uploads/';
$uploads_file = $uploads_dir . basename($_FILES[0]['name']);
$tmp_name = $_FILES[0]['tmp_name'];

 
if (!move_uploaded_file($tmp_name, $uploads_file)) {
    exit_script_on_failure('UPLOAD_ERROR');
} 

echo json_encode(array('message' => 'SUCCESS'));
// this code may be redundant depending on our implementation
// will the php script insert the video or will eric's 
// python script insert the video?
//use a temp userid that is in the database for testing
/*
$conn = connect();
if (is_null($conn)) {
	echo "Failed to connect to the database\n";
	exit;
}

$_SESSION['id'] = 1;
if (!isset($_SESSION['id'])) {
	echo "User is not logged in\n";
	exit;
}


insert_video($conn, $_SESSION['id'], $uploads_file);
*/
?>