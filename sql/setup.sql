CREATE TABLE IF NOT EXISTS User (
    UserID INT PRIMARY KEY,
    Username VARCHAR(255),
    Password VARCHAR(255),
    FirstName VARCHAR(255),
    LastName VARCHAR(255),
    LastIpAddress VARCHAR(16),
    LastLoginTime TIMESTAMP
);

CREATE TABLE IF NOT EXISTS Video (
    VideoID INT PRIMARY KEY,
    NumFrames INT, 
    FramesPerSecond INT, 
    Width INT,
    Height INT,
    FrameID INT REFERENCES Frame 
    --DataID INT REFERENCES OpenFaceData
);


/*
the database assignment specifies that we will be storing eye points
acquired from OpenFace and Fabian Timm's algorithm (4 points total). 
although the assignments do not specify using OpenFace to get these 
points so we will only store points from FT's algorithm
*/
CREATE TABLE IF NOT EXISTS Frame (
    FrameID INT PRIMARY KEY,
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
    DataID INT PRIMARY KEY,
    X REAL,
    Y REAL
);
