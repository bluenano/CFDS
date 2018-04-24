<?php
header("Access-Control-Allow-Origin: *");
header("Content-Type: application/json; charset=UTF-8");

include_once '../shared/database.php';
include_once '../shared/utilities.php';


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

// get id from session or client
$user_id = 1;
$videos = query_videos($conn, $user_id);

if (is_null($videos)) {
    exit_script_on_failure("No videos found");
}

$video_arr = array();
for ($i = 0; $i < count($videos); $i++) {
    $video_id = $videos[0]['videoid'];
    $title = $videos[0]['title'];
    $upload_date = $videos[0]['uploaddate'];
    $video = array('videoid' => $video_id,
                    'title' => $title,
                    'uploaddate' => $upload_date);
    array_push($video_arr, $video);
}

echo json_encode($video_arr);

?>