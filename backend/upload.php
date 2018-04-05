<?php

// security checks
// check that the file is not empty
// check that file name is in english characters, numbers, and _-. symbols
// check that the file name is not larger than 250 characters
// check that the extension is supported

define ("FORMATS", array("mp4", "avi"));

function check_filename($filename) {
	return (bool) ((preg_match("'[^-0-9A-Za-z_\.]+'", $filename)) ? FALSE : TRUE);
}


function check_filename_length($filename) {
	return (bool) ((mb_strlen($filename, "UTF-8")) > 255 ? FALSE : TRUE); 
}


function check_extension($ext) {
	return (bool) ((!in_array($ext, FORMATS)) ? FALSE : TRUE);
}

?>


<?php
define ("SITE_ROOT", realpath(dirname(__FILE__)));

if ($_FILES[0]['error'] != UPLOAD_ERR_OK) {
	echo "File failed to upload correctly\n";
	exit;
}

if ($_FILES[0]['size'] == 0) {
	echo "File is empty\n";
	exit;
}

if (!check_filename_length($_FILES[0]['name'])) {
	echo "File name is too long\n";
	exit;
}


if (!check_filename($_FILES[0]['name'])) {
	echo "Invalid file name\n";
	exit;
}

$ext = end((explode(".", $_FILES[0]['name'])));
if (!check_extension($ext)) {
	echo "File extension is not supported\n";
	exit;
}


$uploads_dir = SITE_ROOT . '/uploads/';
$uploads_file = $uploads_dir . basename($_FILES[0]['name']);
$tmp_name = $_FILES[0]['tmp_name'];

 
if (move_uploaded_file($tmp_name, $uploads_file)) {
    echo "File was uploaded successfully\n";
} else {
    echo "File was not successfully uploaded\n";
}
?>