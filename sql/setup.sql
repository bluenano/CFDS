/*
perform a database query on the server side toverify the
username/password entered (for existing users) matches an appropriate 
username and password in the database.
*/
CREATE TABLE IF NOT EXISTS User (
    UserID INT PRIMARY KEY,
    Username VARCHAR(255),
    Password VARCHAR(255),
    FirstName VARCHAR(255),
    LastName VARCHAR(255),
    LastIpAddress VARCHAR(16),
    LastLoginTime TIMESTAMP
    VideoID INT REFERENCE Video
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
    VideoID INT PRIMARY KEY,
    NumFrames INT, 
    FramesPerSecond INT, 
    Width INT,
    Height INT,
    FrameID INT REFERENCES Frame 
);


/*
the database assignment specifies that we will be storing eye points
acquired from OpenFace and Fabian Timm's algorithm (4 points total). 
although the assignments do not specify using OpenFace to get these 
points so we will only store points from FT's algorithm
*/
CREATE TABLE IF NOT EXISTS Frame (
    --FrameID INT PRIMARY KEY,
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
    Y REAL
);
