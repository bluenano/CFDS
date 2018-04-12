<?php
// utility constants and functions to be used in our project

function exit_script_on_failure($message) {
	echo json_encode(array('message' => $message));
	exit;
}


?>