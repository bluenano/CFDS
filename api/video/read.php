<?php
header("Access-Control-Allow-Origin: *");
header("Content-Type: application/json; charset=UTF-8");

include_once '../shared/utilities.php';
include_once '../config/database.php';

$conn = connect();
if (is_null($conn)) {
   //|| 
   //!isset($_SESSION['id'])) {
    exit_script_on_failure("CONNECTION_ERROR");
}

// get id from $_SESSION or from the http request
// getting the id from the server will make the api not 
// adhere to REST because the request body should contain 
// all the info needed to complete the request
$id = 1;
$videos = query_videos($conn, $id);

if (is_null($videos)) {
    exit_script_on_failure("No videos found");
}

$video_arr = array();
for ($i = 0; $i < count($videos); $i++) {
    $videoid = $videos[0]['videoid'];
    $title = $videos[0]['title'];
    $uploaddate = $videos[0]['uploaddate'];
    $video = array('videoid' => $videoid,
                    'title' => $title,
                    'uploaddate' => $uploaddate);
    array_push($video_arr, $video);
}

echo json_encode($video_arr);

?>