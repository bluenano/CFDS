<?php
include("database_handler.php");

if (!isset($_POST['username']) 
    ||
    !isset($_POST['password']))  {
	echo 0;
	exit;
} 

date_default_timezone_set("America/Los_Angeles");
$conn = connect();
if (is_null($conn)) {
	echo 'Failed to connect to database' . "\n";
	echo 0;
	exit;
}

$username = $_POST['username'];
$password = $_POST['password'];
 
if (!(verifyLogin($conn, $username, $password))) {
	echo 0;
	exit;
}

// need to embed a session id also
$ip = $_SERVER['REMOTE_ADDR'];
$time = date('h:i:s');

?>
