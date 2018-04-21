/*
Install libpq:

    jw@jw-laptop ~/CS160Project/vidproc $ sudo apt install libpq-dev
    #...
    jw@jw-laptop ~/CS160Project/vidproc $ pg_config --includedir
    /usr/include/postgresql
    jw@jw-laptop ~/CS160Project/vidproc $ pg_config --libdir
    /usr/lib/x86_64-linux-gnu
    jw@jw-laptop ~/CS160Project/vidproc $ pkg-config --cflags libpq
    -I/usr/include/postgresql
    jw@jw-laptop ~/CS160Project/vidproc $ pkg-config --libs libpq
    -lpq

# build this program
    g++ -std=c++11 -Wall -Wextra -O2 dbtest.cpp -l pq -s -o dbtest.out
*/


#if 0

CREATE TABLE IF NOT EXISTS userinfo (
    userid SERIAL PRIMARY KEY,
    username VARCHAR(255),
    password CHAR(60),
    firstname VARCHAR(255),
    lastname VARCHAR(255),
    lastip VARCHAR(16),
    lastlogin TIMESTAMP,
    sessionid VARCHAR(128)
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
    title VARCHAR(255),
    uploaddate DATE,
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
    frameid SERIAL PRIMARY KEY,
    videoid SERIAL REFERENCES video(videoid),
    framenumber INT,
    ftpupilrightx INT,
    ftpupilrighty INT,
    ftpupilleftx INT,
    ftpupillefty INT,
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
    x INT,
    y INT,
    frameid INT REFERENCES frame(frameid)
);
#endif

//std includes
#include <stdio.h>
#include <stdlib.h>
//third party,
//using this over pqxx because seems simpler to install
//and pqxx probably just wraps this
#include <postgresql/libpq-fe.h>//depending on how you installed may need to only put #include <libpq-fe.h>
                                //or compile with -I path/to/folder/countaning/postgresql # <- directory
//ours
#include "db.h"

/*
    There is room for improvement but this is simple.
    Using given schema.
    There is something called a prepared statement,
    but even that doesn't seem as streamlined as could be.
*/

/*
    Table "frame" contains eye coords and pose for a single frame.

    Table "openfacedata" (we're using dlib, not openface) holds only 1 of the 68 facial landmarks
    for a given frame... so I do 68 inserts in a loop per 1 frame? Are they kept together? <-doesnt seem like it.

    I'm new to databases and SQL, but is it possible to just store a fixed length 
    binary blob of length sizeof(FrameResults) ?
*/

static
void exit_nicely(PGconn *conn)
{
    PQfinish(conn);
    exit(1);
}

static
char buf[1024];

static//yaw, pitch, roll (in some kind of order) and coords of both pupils
void insert_frame(PGconn *const conn, const FrameResults& all)
{
    sprintf(buf,
        "INSERT INTO frame("
        "videoid, "
        "framenumber, "
        "ftpupilrightx, "
        "ftpupilrighty, "
        "ftpupilleftx, "
        "ftpupillefty, "
        "roll, "//not ordered yaw, pitch, roll in schema??????
        "pitch, "//give explicit value names 
        "yaw)"
        "VALUES (%d, %u, %u, %u, %u, %u, %f, %f, %f)",//I tested it and it seems to work without semi colons.
        -1,//video id
        all.frameno,
        all.right_pupil.x16,
        all.right_pupil.y16,
        all.left_pupil.x16,
        all.left_pupil.y16,
        all.rotation.roll,
        all.rotation.pitch,
        all.rotation.yaw);
    puts(buf);
    //yo dawg I heard you like parsing so I'll parse
    //the fmt string to figure out how you want this printed,
    //and then the db will parse what I just printed to get the actual values.

    PGresult *res = PQexec(conn, buf);

    if (res == NULL)//dont know if I check against this or call a function on ptr like did with PQconnectdb, not documented clearly.
    {
        fprintf(stderr, "PQexec failed: %s", PQerrorMessage(conn));
        PQclear(res);
        exit_nicely(conn);
    }

    PQclear(res);//after every exec
}

static//"openface data"
void insert_marks68(PGconn *const conn, const FrameResults& all)
{
    static
    const char prefix[] = "INSERT INTO openfacedata(pointnumber, x, y, frameid) VALUES (";

    //memcpy, strcpy, strcat all have trash return values... give me a pointer to the end
    int isuf = 0;
    for ( ; prefix[isuf]!='\0'; ++isuf)
        buf[isuf] = prefix[isuf];
    char *const suffix = buf + isuf;

    //XXX: Are we doing zero-based indexing? thats what I'm doing here.
    for (int i=0; i<68; ++i)
    {
        sprintf(suffix, "%u, %u, %u, %u)",//no semicolon
          i,//point id in [0, 68) for this frame
          all.marks68[i].x16,
          all.marks68[i].y16,
          0);//frame id

        if (i==10)
            puts(buf);

        PGresult *res = PQexec(conn, buf);
        if (res == NULL)
        {
            fprintf(stderr, "Last PQexec failed: %s", PQerrorMessage(conn));
            PQclear(res);
            exit_nicely(conn);
        }
        PQclear(res);//after every exec
    }
}

int main()
{
    puts("DB test program");
	PGconn *const conn = PQconnectdb("user=postgres host=localhost password=postgres dbname=cs160");
    if (PQstatus(conn) != CONNECTION_OK)
    {
        fprintf(stderr, "Connection to database failed: %s", PQerrorMessage(conn));
        exit_nicely(conn);
    }

    FrameResults all = {};

    //@TODO XXX:
    //somebody else deal with this and fit it into the schema.
    VideoMetadata metadata;//will be unused and issue warnings as a reminder.
    metadata.nframes = 0; //NB: can't be properly determined until all processing is done
    metadata.fps = 0;
    metadata.width = 0;
    metadata.height = 0;

    for (int i=0; i<68; ++i)
        all.marks68[i] = { (int16_t)i, (int16_t)i };

    all.left_pupil = { 5, 5 };
    all.right_pupil = { 7, 7 };

    all.rotation = { 0.25f, 0.50f, 0.75f };

    all.frameno = 0;

    insert_frame(conn, all);
    insert_marks68(conn, all);

    PQfinish(conn);
    return 0;
}