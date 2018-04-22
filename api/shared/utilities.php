<?php

// utility constants and functions to be used in our project

// supported media formats
define ('FORMATS', array('avi', 'mp4'));


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


function validate($path) {
    $cmd = "ffmpeg -v error -i $path -map 0:1 -f null - 2>error.log";
    shell_exec($cmd); 
    $contents = file_get_contents('error.log');
    return (strlen($contents) == 0 ? TRUE : FALSE);
}


function parse_fps($ffprobeOut) {
    $split = explode('/', $ffprobeOut);
    return $split[0];
}


function parse_resolution($ffprobeOut) {
    $split = explode("\n", $ffprobeOut);
    $width = explode("=", $split[0]);
    $height = explode("=", $split[1]);
    return array($width[1], $height[1]);
}
?>