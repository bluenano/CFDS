<?php

include_once '../shared/database.php';
include_once '../shared/utilities.php';

if (!isset($_POST['username'])
    ||
    !isset($_POST['password'])) {
    exit_script_on_failure('POST_FAILURE');
}


$user_name = $_POST['username'];
$password = $_POST['password'];

$first_name = isset($_POST['firstname']) ? $_POST['firstname'] : null;
$last_name = isset($_POST['lastname']) ? $_POST['lastname'] : null;

$conn = connect();
if (is_null($conn)) {
    exit_script_on_failure('CONNECTION_FAILURE');
}


if (!create_new_user($conn, $user_name, $password, $first_name, $last_name)) {
    exit_script_on_failure('USER_FAILURE');
}

echo json_encode(array('success' => TRUE));


?>