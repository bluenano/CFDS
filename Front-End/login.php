<?php
if (isset( $_POST['username'] ) ) {
	echo "username is " . $_POST['username'] . "\n";
} else {
	echo "username is not set\n";
}

if (isset( $_POST['password'] ) ) {
   	echo "password is " . $_POST['password'] . "\n";
} else {
   	echo "password is not set\n";
}
?>
