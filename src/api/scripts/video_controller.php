<?php
// implement a controller that handles the different
// requests involving reading or deleting a video

include_once '../shared/utilities.php';

// check that operation is set
// expect frontend to send: read or delete operation
// for read: user_id is sent
// for delete: video_id is sent maybe user_id should be sent too

if (!isset($_GET['op'])
    ||
    !isset($_GET['id'])) {
    exit_script_on_failure('GET_FAILURE');
}

$operation = $_GET['op'];
$id = $_GET['id'];


if (str_cmp($operation, "read")) {
    $script = "php ../video/read.php $id";
} else if (str_cmp($operation, "read_all")) {
    $script = "php ../video/read_all.php $id";
} else if (str_cmp($operation, "delete")) {
    $script = "php ../video/delete.php $id";
} else {
    exit_script_on_failure("OPERATION_ERROR");
}

exec($script);

?>