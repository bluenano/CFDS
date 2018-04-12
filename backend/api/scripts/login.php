<?php
session_start();
include_once '../shared/utilities.php';

function end_session_and_exit($message) {
    session_unset();
    session_destroy();
    exit_script_on_failure($message);
}

?>


<?php
include_once '../config/database.php';

if (!isset($_POST['username']) 
    ||
    !isset($_POST['password']))  {
    end_session_and_exit('POST_FAILURE');
} 

date_default_timezone_set('America/Los_Angeles');


$conn = connect();
if (is_null($conn)) {
    end_session_and_exit('CONNECTION_FAILURE');
}

$username = $_POST['username'];
$password = $_POST['password'];
if (!(verify_login($conn, $username, $password))) {
    end_session_and_exit('LOGIN_FAILURE');
}


$_SESSION['id'] = query_user_id($conn, $username);
if (is_null($_SESSION['id'])) {
    end_session_and_exit('USERID_FAILURE');
}


$session = create_session_id($conn, $_SESSION['id']);
if (!$session) {
    end_session_and_exit('SESSIONID_FAILURE');
}


$ip = $_SERVER['REMOTE_ADDR'];
$time = date('Y-m-d H:i:s');
if (!update_after_login($conn, $_SESSION['id'], $ip, $time)) {
    end_session_and_exit('UPDATE_FAILURE');
}


// if client receives a session, then login was successful
// load user account page from here
$send = (array('SUCCESS', $session));
echo json_encode(array('message' => 'SUCCESS',
                       'sessionid' => $session)); 

  
?>