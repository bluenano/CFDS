<?php

// utility constants and functions to be used in our project

define ('SITE_ROOT', '~/Sites/cs160/test');
define ('FORMATS', array('mp4', 'avi', 'mov'));

function exit_script_on_failure($message) {
    echo json_encode(array('message' => $message));
    exit;
}

function end_session_and_exit($message) {
    session_unset();
    session_destroy();
    exit_script_on_failure($message);
}


// validate upload files
function check_filename($filename) {
	return (bool) ((preg_match("'[^-0-9A-Za-z_\.]+'", $filename)) ? FALSE : TRUE);
}


function check_filename_length($filename) {
	return (bool) ((mb_strlen($filename, 'UTF-8')) > 255 ? FALSE : TRUE); 
}


function check_extension($ext) {
	return (bool) ((!in_array($ext, FORMATS)) ? FALSE : TRUE);
}

?>