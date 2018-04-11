<?php
session_start();

function end_session() {
    session_unset();
    session_destroy();
}

function exit_script_on_failure($error) {
    end_session();
    echo json_encode($error);
    exit;
}
?>


<?php
include('database_handler.php');

if (!isset($_POST['username']) 
    ||
    !isset($_POST['password']))  {
    exit_script_on_failure('POST_FAILURE');
} 

date_default_timezone_set('America/Los_Angeles');


$conn = connect();
if (is_null($conn)) {
    exit_script_on_failure('CONNECTION_FAILURE');
}

$username = $_POST['username'];
$password = $_POST['password'];
if (!(verify_login($conn, $username, $password))) {
    exit_script_on_failure('LOGIN_FAILURE');
}


$_SESSION['id'] = query_user_id($conn, $username);
if (is_null($_SESSION['id'])) {
    exit_script_on_failure('USERID_FAILURE');
}


$session = create_session_id($conn, $_SESSION['id']);
if (!$session) {
    exit_script_on_failure('SESSIONID_FAILURE');
}


$ip = $_SERVER['REMOTE_ADDR'];
$time = date('Y-m-d H:i:s');
if (!update_after_login($conn, $_SESSION['id'], $ip, $time)) {
    exit_script_on_failure('UPDATE_FAILURE');
}


// if client receives a session, then login was successful
// load user account page from here
$send = (array('SUCCESS', $session));
echo json_encode($send); 

  
?>
