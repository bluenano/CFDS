<?php
session_start();

function end_session() {
    session_unset();
    session_destroy();
}

function exit_script_on_failure() {
    end_session();
    echo 0;
    exit;
}
?>


<?php
include('database_handler.php');

if (!isset($_POST['username']) 
    ||
    !isset($_POST['password']))  {
    exit_script_on_failure();
} 


date_default_timezone_set("America/Los_Angeles");

$conn = connect();
if (is_null($conn)) {
    echo 'Failed to connect to database' . "\n";
    exit_script_on_failure();
}

$username = $_POST['username'];
$password = $_POST['password'];
 
if (!(verify_login($conn, $username, $password))) {
    echo "Failed to login\n";
    exit_script_on_failure();
}


$_SESSION['id'] = query_user_id($conn, $username);

if (is_null($_SESSION['id'])) {
    echo "Failed to set user id\n";
    exit_script_on_failure();
}


$session = create_session_id($conn, $_SESSION['id']);
if (!$session) {
    echo "Failed to create a session id\n";
    exit_script_on_failure();
}


$ip = $_SERVER['REMOTE_ADDR'];
$time = date('Y-m-d H:i:s');
if (!update_after_login($conn, $_SESSION['id'], $ip, $time)) {
    echo "Failed to update database after login\n";
    exit_script_on_failure();
}


// if client receives a session, then login was successful
// load user account page from here
echo $session;   
?>
