<?php
function exit_script_on_failure($error) {
    echo json_encode($error);
    exit;
}
?>

<?php
include('database_handler.php');


if (!isset($_POST['username'])
    ||
    !isset($_POST['password'])) {
    exit_script_on_failure('POST_FAILURE');
}


$username = $_POST['username'];
$password = $_POST['password'];

$firstname = isset($_POST['firstname']) ? $_POST['firstname'] : null;
$lastname = isset($_POST['lastname']) ? $_POST['lastname'] : null;

$conn = connect();
if (is_null($conn)) {
    exit_script_on_failure('CONNECTION_FAILURE');
}


if (!create_new_user($conn, $username, $password, $firstname, $lastname)) {
    exit_script_on_failure('USER_FAILURE');
}

echo json_encode('SUCCESS');


?>