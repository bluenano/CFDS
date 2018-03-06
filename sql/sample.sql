CREATE TABLE users (
    Username char,
    Password char,
    FirstName char,
    LastName char,
    LastIpAddress char,
    LastLoginTime TIMESTAMP,
    VideoID INT REFERENCES Video
);

CREATE TABLE Video (
    VideoID INT PRIMARY Key,
    NumFrames int, 
    FramesPerSecond real, 
    Width int,
    Height INT,
    FrameID INT REFERENCES Frame, 
    DataID INT REFERENCES OpenFaceData
);

CREATE TABLE Frame (
    FrameID INT PRIMARY Key,
    FT_PupilRight point,
    FT_PupilLeft point,
    Pupil_RightOf point,
    Pupil_LeftOf point,
    Roll real,
    Pitch real,
    Yaw REAL
);

CREATE TABLE OpenFaceData (
    DataID INT PRIMARY Key,
    X point,
    Y point
);
