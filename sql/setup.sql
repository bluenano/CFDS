/*
You can run the script by first making sure the postgres server
is running and by starting psql and connecting to a specified
database. 

Then in psql, run \i path_to_setup.sql 
*/

CREATE TABLE IF NOT EXISTS UserInfo (
    UserID SERIAL PRIMARY KEY,
    Username VARCHAR(255),
    Password CHAR(60),
    FirstName VARCHAR(255),
    LastName VARCHAR(255),
    LastIp VARCHAR(16),
    LastLogin TIMESTAMP
);

/*
Your program should extract and store metadata about the video in your database. 
The metadata that should be stored.The number of frames in the video.  
The frames per second (fps) or frame rate the video was encoded at.    
The horizontal (X) and vertical (Y) pixel resolution of the video.    
A unique ID (primary key) for this video  
The login username (i.e. registered user) of the person who uploaded this video.
*/
CREATE TABLE IF NOT EXISTS Video (
    VideoID SERIAL PRIMARY KEY,
    UserID SERIAL REFERENCES UserInfo(UserID),
    NumFrames INT, 
    FramesPerSecond INT, 
    Width INT,
    Height INT
);


/*
the database assignment specifies that we will be storing eye points
acquired from OpenFace and Fabian Timm's algorithm (4 points total). 
although the assignments do not specify using OpenFace to get these 
points so we will only store points from FT's algorithm
*/
CREATE TABLE IF NOT EXISTS Frame (
    VideoID SERIAL REFERENCES Video(VideoID),
    FTPupilRightX REAL,
    FTPupilRightY REAL,
    FTPupilLeftX REAL,
    FTPupilLeftY REAL,
    Roll REAL,
    Pitch REAL,
    Yaw REAL
);

/*
the key for open face data should be which point
it is and the FrameID associated with that point.
recall that open face will return 68 points if a face
is found in an image 
*/
CREATE TABLE IF NOT EXISTS OpenFaceData (
    X REAL,
    Y REAL,
    VideoID SERIAL REFERENCES Video(VideoID)
);
