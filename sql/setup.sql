CREATE TABLE IF NOT EXISTS Users (
    Username VARCHAR(255),
    Password VARCHAR(255),
    FirstName VARCHAR(255),
    LastName VARCHAR(255),
    LastIpAddress VARCHAR(16),
    LastLoginTime TIMESTAMP
);

CREATE TABLE IF NOT EXISTS Video (
    VideoID INT PRIMARY Key,
    NumFrames INT, 
    FramesPerSecond INT, 
    Width INT,
    Height INT,
    FrameID INT REFERENCES Frame 
    --DataID INT REFERENCES OpenFaceData
);

CREATE TABLE IF NOT EXISTS Frame (
    FrameID INT PRIMARY Key,
    PupilRightX REAL,
    PupilRightY REAL,
    PupilLeftX REAL,
    PupilLeftY REAL,
    Roll REAL,
    Pitch REAL,
    Yaw REAL
);

CREATE TABLE IF NOT EXISTS OpenFaceData (
    DataID INT PRIMARY Key,
    X REAL,
    Y REAL
);
