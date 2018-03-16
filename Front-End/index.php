<?php
if (isset( $_POST['username'] ) ) {
	echo "php: username is " . $_POST['username'] . "\n";
} else {
	echo "username is not set\n";
}

if (isset( $_POST['email'] ) ) {
	echo "php: email is " . $_POST['email'] . "\n";
} else {
	echo "email is not set\n";
}

if (isset( $_POST['email'] ) ) {
	echo "php: first name is " . $_POST['firstname'] . "\n";
} else {
	echo "first name is not set\n";
}

if (isset( $_POST['email'] ) ) {
	echo "php: last name is " . $_POST['lastname'] . "\n";
} else {
	echo "last name is not set\n";
}

if (isset( $_POST['password'] ) ) {
   	echo "php: password is " . $_POST['password'] . "\n";
} else {
   	echo "password is not set\n";
}
?>