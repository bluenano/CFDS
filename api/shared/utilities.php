<?php

// utility functions to be used in our project
include_once '../../../config.php';
 
function exit_script_on_failure($message) {
    echo json_encode(array('success' => FALSE,
                           'message' => $message));
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


// valid length is <= 255
function check_filename_length($filename) {
    return (bool) ((mb_strlen($filename, 'UTF-8')) > 255 ? FALSE : TRUE); 
}


function check_extension($ext) {
    return (bool) ((!in_array($ext, FORMATS)) ? FALSE : TRUE);
}


// validate a video
function validate($path) {
    $cmd = "ffmpeg -v error -i $path -map 0:1 -f null - 2>error.log";
    shell_exec($cmd); 
    $contents = file_get_contents('error.log');
    return (strlen($contents) == 0 ? TRUE : FALSE);
}

?>