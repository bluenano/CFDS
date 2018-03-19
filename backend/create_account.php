<?php
function exit_script_on_failure() {
    echo 0;
    exit;
}
?>

<?php
include("database_handler.php");


if (!isset($_POST['username'])
    OR
    !isset($_POST['password'])) {
    exit_script_on_failure();
}


$username = $_POST['username'];
$password = $_POST['username'];
$firstname = isset($_POST['firstname']) ? $_POST['firstname'] : null;
$lastname = isset($_POST['lastname']) ? $_POST['lastname'] : null;

$conn = connect();
if (is_null($conn)) {
    echo "Failed to connect to database\n";
    exit_script_on_failure();
}


if (!create_new_user($conn, $username, $password, $firstname, $lastname)) {
    echo "Failed to create a new user. Username may be taken.\n";
    exit_script_on_failure();
}

echo "Successfully created a new user.\n";

// fields are username, email, firstname, lastname, password
// I am not going to handle the email because we do not
// have a field for it in our database


?>