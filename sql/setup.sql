/*
You can run the script by first making sure the postgres server
is running and by starting psql and connecting to a specified
database. 

Then in psql, run \i path_to_setup.sql 
*/

CREATE TABLE IF NOT EXISTS userinfo (
    userid SERIAL PRIMARY KEY,
    username VARCHAR(255),
    password CHAR(60),
    firstname VARCHAR(255),
    lastname VARCHAR(255),
    lastip VARCHAR(16),
    lastlogin TIMESTAMP,
    sessionid CHAR(32)
);

/*
Your program should extract and store metadata about the video in your database. 
The metadata that should be stored.The number of frames in the video.  
The frames per second (fps) or frame rate the video was encoded at.    
The horizontal (X) and vertical (Y) pixel resolution of the video.    
A unique ID (primary key) for this video  
The login username (i.e. registered user) of the person who uploaded this video.
*/
CREATE TABLE IF NOT EXISTS video (
    videoid SERIAL PRIMARY KEY,
    userid SERIAL REFERENCES userinfo(userid),
    numframes INT, 
    framespersecond INT, 
    width INT,
    height INT,
    videopath VARCHAR(255)
);


/*
the database assignment specifies that we will be storing eye points
acquired from OpenFace and Fabian Timm's algorithm (4 points total). 
although the assignments do not specify using OpenFace to get these 
points so we will only store points from FT's algorithm
*/
CREATE TABLE IF NOT EXISTS frame (
    videoid SERIAL REFERENCES video(videoid),
    framenumber INT,
    ftpupilrightx REAL,
    ftpupilrighty REAL,
    ftpupilleftx REAL,
    ftpupillefty REAL,
    roll REAL,
    pitch REAL,
    yaw REAL
);

/*
the key for open face data should be which point
it is and the FrameID associated with that point.
recall that open face will return 68 points if a face
is found in an image 
*/
CREATE TABLE IF NOT EXISTS openfacedata (
    pointnumber INT,
    x REAL,
    y REAL,
    videoid SERIAL REFERENCES video(videoid)
);
