<?php
define ('SITE_ROOT', realpath(dirname(__FILE__)));

$uploads_dir = SITE_ROOT . '/uploads/';
$uploads_file = $uploads_dir . basename($_FILES[0]['name']);
$tmp_name = $_FILES[0]['tmp_name'];
 
 
if (move_uploaded_file($tmp_name, $uploads_file)) {
    echo "File was uploaded successfully\n";
} else {
    echo "File was not successfully uploaded\n";
}
?>