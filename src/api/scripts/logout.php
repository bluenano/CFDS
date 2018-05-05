<?php
session_start();
// implement the ability to log a user out

include_once '../shared/database.php';
include_once '../shared/utilities.php';

// clear the session id in the database
$conn = connect();
if (is_null($conn)) {
    end_session_and_exit("CONNECTION_ERROR");
}

// get user_id from request
$user_id = 6;
if (!clear_session($conn, $user_id)) {
    end_session_and_exit("REMOVE_SESSION_ERROR");
}

echo json_encode(array('success' => TRUE));
end_session();

?>