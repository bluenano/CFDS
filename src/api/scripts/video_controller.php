<?php
// implement a controller that handles the different
// api requests involving reading or deleting a video
// read, delete 

include_once '../shared/utilities.php';

// check that operation is set
// expect frontend to send: read or delete operation
// for read: user_id is sent
// for delete: video_id is sent maybe user_id should be sent too

if (!isset($_POST['json'])) {
    exit_script_on_failure('POST_FAILURE');
}

$json = json_decode($_POST['json']);
$operation = $json['operation'];
$id = $json['id'];


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