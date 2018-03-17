<?php
session_start();

function end_session() {
	session_unset();
	session_destroy();
}

function exit_script() {
	end_session();
	echo 0;
	exit;
}

?>

<?php
include("database_handler.php");

if (!isset($_POST['username']) 
    ||
    !isset($_POST['password']))  {
	exit_script();
} 

date_default_timezone_set("America/Los_Angeles");

$conn = connect();
if (is_null($conn)) {
	echo 'Failed to connect to database' . "\n";
	exit_script();
}

$username = $_POST['username'];
$password = $_POST['password'];
 
if (!(verify_login($conn, $username, $password))) {
	exit_script();
}

// could store the id from the db to pass to other php scripts
$_SESSION['username'] = $username;

$session = insert_session_id($conn, $username);
$ip = $_SERVER['REMOTE_ADDR'];
$time = date('Y-m-d H:i:s');
update_after_login($conn, $username, $ip, $time);


// echo the session back to client
// if client receives a session, then login was successful
// load user account page
echo $session . "\n";	
?>
