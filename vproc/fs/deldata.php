<?php
/*

    jw@jw-laptop ~/CS160Project/vproc/fs $ make
    g++ -std=c++11 -Wall -Wextra -DHOME="/home/jw" -O1 -s fs.cpp -o fs.out
    jw@jw-laptop ~/CS160Project/vproc/fs $ ./fs.out 777
    making file with video id: 777
    Opening "wb":
      [/home/jw/Sites/cs160/vdata/vdata_777.dat]
    file filled and closed successfully
    jw@jw-laptop ~/CS160Project/vproc/fs $ ls -l ~/Sites/cs160/vdata/vdata_777.dat 
    -rw-rw-r-- 1 jw jw 1476 May  8 12:24 /home/jw/Sites/cs160/vdata/vdata_777.dat
    jw@jw-laptop ~/CS160Project/vproc/fs $ ./fs.out 
    enter a file to inspect: /home/jw/Sites/cs160/vdata/vdata_777.dat
    looking for file:
      [/home/jw/Sites/cs160/vdata/vdata_777.dat]
    nframes: 5,
    fps: 30,
    w: 400,
    h: 300
    enter a valid 0-indexed frame to inspect, i for next sequential, or q to break
    >> 3
    203,203   0,  0   0,  0   0,  0   0,  0   0,  0   0,  0   0,  0
      0,  0   0,  0   0,  0   0,  0   0,  0   0,  0   0,  0   0,  0
      0,  0   0,  0   0,  0   0,  0   0,  0   0,  0   0,  0   0,  0
      0,  0   0,  0   0,  0   0,  0   0,  0   0,  0   0,  0   0,  0
      0,  0   0,  0   0,  0   0,  0   0,  0   0,  0   0,  0   0,  0
      0,  0   0,  0   0,  0   0,  0   0,  0   0,  0   0,  0   0,  0
      0,  0   0,  0   0,  0   0,  0   0,  0   0,  0   0,  0   0,  0
      0,  0   0,  0   0,  0   0,  0   0,  0   0,  0   0,  0   0,  0
      0,  0   0,  0   0,  0   0,  0
       ... are the (x,y) 68 landmark coords.
    left:  { -1,  -1}
    right: {  5,   3}
    y,p,r: {0.250000, 0.500000, 0.750000}
    >> q
    enter a file to inspect: ^Z
    [5]+  Stopped                 ./fs.out
    jw@jw-laptop ~/CS160Project/vproc/fs $ php -f deldata.php 777
    attempting to unlink: /home/jw/Sites/cs160/vdata/vdata_777.dat
    file deleted 
    jw@jw-laptop ~/CS160Project/vproc/fs $ ls -l ~/Sites/cs160/vdata/vdata_777.dat 
    ls: cannot access '/home/jw/Sites/cs160/vdata/vdata_777.dat': No such file or directory
    jw@jw-laptop ~/CS160Project/vproc/fs $ 

*/

function delete_vdata_for_id($id) //******** @sean can put this function in your code to call when user deletes a video
{

    $homestr = exec('echo $HOME');//need be single quotes? also probably a better way

    $file = $homestr."/Sites/cs160/vdata/vdata_".$id.".dat";
    
    echo "attempting to unlink: ".$file."\n";

    if (!unlink($file))
    {
        echo "unlink() error: ".posix_strerror(posix_get_last_error());
    }
    else
    {
        echo "file deleted";
    }
}

//main

if ($argc !== 2)
{
    echo "Need to supply single commandline video id argument\n.";
}
else
{
    delete_vdata_for_id($argv[1]);
}   


?> 
