//$ prog.out [video_id=1]
//assumes a row in video table with that primary key already exists
//See tail of file for sample output,
//which accomponies dbpsqltest.txt

/*
Install libpq:

    jw@jw-laptop ~/CS160Project/vidproc $ sudo apt install libpq-dev

Build this program:
    g++ -std=c++11 -Wall -Wextra -O1 dbtest.cpp -l pq -s -o dbtest.out

You may have to tweak the above, by like adding flags such as -I <incdir> or -L <libdir>
Some information might be queried with these commands:
    jw@jw-laptop ~/CS160Project/vidproc $ pg_config --includedir
    /usr/include/postgresql
    jw@jw-laptop ~/CS160Project/vidproc $ pg_config --libdir
    /usr/lib/x86_64-linux-gnu
    jw@jw-laptop ~/CS160Project/vidproc $ pkg-config --cflags libpq
    -I/usr/include/postgresql
    jw@jw-laptop ~/CS160Project/vidproc $ pkg-config --libs libpq
    -lpq

Also note that the postgres headers are #included as <postgresql/libpq-fe.h>
not <libpq-fe.h>
*/


#if 0
-- ***************************** The "root" table
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
    userid SERIAL REFERENCES userinfo(userid), -- **************** should this be serial?
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
    frameid SERIAL PRIMARY KEY, -- **** I start at 1 and go to nframes inclusive? Bother with RETURNING?
    -- wait... apparently each frame is its own distinct row, and not attributed to any
    videoid SERIAL REFERENCES video(videoid), -- *********** should this be serial?
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
    frameid INT REFERENCES frame(frameid) -- *****************************
);
#endif

//std
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//others
#include <postgresql/libpq-fe.h>
//ours
#include "db.h"

/*
    Table "frame" contains eye coords and pose for a single frame.

    Each row of table "openfacedata" (we're using dlib, not openface)
    holds only 1 of the 68 facial landmarks.
    Do 68 inserts in a row, each of those will have ref to to frame(frameid).
*/

//idk if need for PGresult
static inline
uint32_t unaligned_load_u32(const void* p)
{
    uint32_t v;
    memcpy(&v, p, sizeof(uint32_t));
    return v;
}

static
void exit_nicely(PGconn *conn)
{
    PQfinish(conn);
    exit(1);
}

static
bool res_ok(const PGresult* r)
{
    ExecStatusType const v = PQresultStatus(r); 
    return v==PGRES_COMMAND_OK || v==PGRES_TUPLES_OK;//which one in case of RETURNING?
}

static
PGresult* exec_success_or_die(PGconn* conn, const char* szcommand)
{
    PGresult *res = PQexec(conn, szcommand);

    if (! res_ok(res))
    {
        fprintf(stderr, "PQexec error: %s\n", PQresultErrorMessage(res));
        PQclear(res);
        exit_nicely(conn);
    }

    return res;//caller must call PQclear
}

//returns frameid
static//yaw, pitch, roll (in some kind of order) and coords of both pupils
unsigned insert_frame(PGconn *const conn, const FrameResults& all, unsigned videoid)
{
    char buf[1024];
    sprintf(buf, "INSERT INTO frame("
        //serial frameid pkey
        "videoid, "
        "framenumber, "
        "ftpupilrightx, "
        "ftpupilrighty, "
        "ftpupilleftx, "
        "ftpupillefty, "
        "roll, "//not ordered yaw, pitch, roll in schema?
        "pitch, "//give explicit value names 
        "yaw)"
        "VALUES (%u, %u, %u, %u, %u, %u, %f, %f, %f) "
        "RETURNING frameid",//this is a mess...
        videoid,
        all.frameno,
        all.right_pupil.x16,
        all.right_pupil.y16,
        all.left_pupil.x16,
        all.left_pupil.y16,
        all.rotation.roll,
        all.rotation.pitch,
        all.rotation.yaw);
    puts(buf);

    PGresult *res = exec_success_or_die(conn, buf);
    //okay... apparently not binary, but text.
    #if 0
        //XXX: what is the endianess???
        const char * bin = PQgetvalue(res, 0, 0);
        //unsigned const netl = unaligned_load_u32(bin);
        PQclear(res);//after every exec
        //unsigned const myorder = __builtin_bswap32(netl);//or do ntohl // apparently don't do this
        //printf("returned frameid: %u\n", netl);
        //return myorder;
        unsigned netl = atoi(bin);
    #endif
    unsigned const fid = atoi(PQgetvalue(res, 0, 0));
    PQclear(res);
    return fid;
}
        //The binary representation of INT4 is in network byte order, which
        //we'd better coerce to the local byte order.
        //ival = ntohl(*((uint32_t *) iptr));

static//"openface data"
void insert_marks68(PGconn *const conn, const FrameResults& all, unsigned frameid)
{
    char buf[1024];

    static
    const char prefix[] = "INSERT INTO openfacedata(pointnumber, x, y, frameid) VALUES (";

    //copy while getting ptr to end
    int iend = 0;
    for ( ; prefix[iend]!='\0'; ++iend)
        buf[iend] = prefix[iend];
    char *const suffix = buf + iend;

    //It seems pointnumber is not a key and could start at anything,
    //but I'll make it start at 1 for consistency as postgres seems to like that.
    for (int i=0; i<68; ++i)
    {
        sprintf(
            suffix,
            "%u, %u, %u, %u)",//no semicolon
            i+1,
            all.marks68[i].x16,
            all.marks68[i].y16,
            frameid);
        if (i==0 || i==34 || i==67)
            putchar('\n'),
            puts(buf);
        else
            putchar('*');

        PGresult *res = exec_success_or_die(conn, buf); 

        PQclear(res);//after every exec
    }
}

static
void update_video(PGconn *conn, const VideoMetadata& vdata, unsigned videoid)
{
    char buf[1024];
    sprintf(
        buf,
        "UPDATE video SET (numframes, framespersecond, width, height) = (%u, %u, %u, %u) WHERE videoid = %u",
        vdata.nframes, vdata.fps, vdata.width, vdata.height, videoid);
    puts(buf);

    PGresult *res = exec_success_or_die(conn, buf); 

    PQclear(res);//after every exec
}

#define NFRAMES 3

//$ prog.out [video_id=1]
//assumes a row in video with that primary key already exists
int main(int argc, char** argv)
{
    unsigned videoid = 1u;
    if (argc==2)
    {
        unsigned const x = atoi(argv[1]);
        if (x==0u || x >0xffffu)
            fputs("videoid must be in [1, 0xffff], using default of 1\n", stderr);
        else
            videoid = x;
    }
    printf("videoid: %u\n", videoid);

	PGconn *const conn = PQconnectdb("user=postgres host=localhost password=postgres dbname=cs160");//@TODO: will need another config option for vidproc exe...
    if (PQstatus(conn) != CONNECTION_OK)
    {
        fprintf(stderr, "Connection to database failed: %s", PQerrorMessage(conn));
        exit_nicely(conn);
    }

    puts("Connected to db");

    FrameResults all = {};

    for (int i=0; i!=68; ++i)//will be same for each NFRAMES
        all.marks68[i] = { (int16_t)i, (int16_t)i };

    all.left_pupil  = { 200,     480/3 };
    all.right_pupil = { 640-200, 480/3 };

    all.rotation = { 0.25f, 0.50f, 0.75f };

    for (int ii=1; ii<=NFRAMES; ++ii)//NB: start 1 and inclusive
    {
        all.frameno = ii;//I geuss start at one for postgres consistency?

        unsigned const frameid = insert_frame(conn, all, videoid);//SQL 1
        insert_marks68(conn, all, frameid);//SQL 2
    }

    VideoMetadata metadata;
    metadata.nframes = NFRAMES;//NB: can't be properly determined until all processing is done
    metadata.fps = 30;
    metadata.width = 640;
    metadata.height = 480;

    update_video(conn, metadata, videoid);//SQL 3

    PQfinish(conn);
    return 0;
}


#if 0
#Ran it once before with videoid=1, not shown. Also that was after psql stuff.

jw@jw-laptop ~/CS160Project/vidproc $ ./dbtest.out 2
videoid: 2
Connected to db
INSERT INTO frame(videoid, framenumber, ftpupilrightx, ftpupilrighty, ftpupilleftx, ftpupillefty, roll, pitch, yaw)VALUES (2, 1, 440, 160, 200, 160, 0.750000, 0.500000, 0.250000) RETURNING frameid

INSERT INTO openfacedata(pointnumber, x, y, frameid) VALUES (1, 0, 0, 4)
*********************************
INSERT INTO openfacedata(pointnumber, x, y, frameid) VALUES (35, 34, 34, 4)
********************************
INSERT INTO openfacedata(pointnumber, x, y, frameid) VALUES (68, 67, 67, 4)
INSERT INTO frame(videoid, framenumber, ftpupilrightx, ftpupilrighty, ftpupilleftx, ftpupillefty, roll, pitch, yaw)VALUES (2, 2, 440, 160, 200, 160, 0.750000, 0.500000, 0.250000) RETURNING frameid

INSERT INTO openfacedata(pointnumber, x, y, frameid) VALUES (1, 0, 0, 5)
*********************************
INSERT INTO openfacedata(pointnumber, x, y, frameid) VALUES (35, 34, 34, 5)
********************************
INSERT INTO openfacedata(pointnumber, x, y, frameid) VALUES (68, 67, 67, 5)
INSERT INTO frame(videoid, framenumber, ftpupilrightx, ftpupilrighty, ftpupilleftx, ftpupillefty, roll, pitch, yaw)VALUES (2, 3, 440, 160, 200, 160, 0.750000, 0.500000, 0.250000) RETURNING frameid

INSERT INTO openfacedata(pointnumber, x, y, frameid) VALUES (1, 0, 0, 6)
*********************************
INSERT INTO openfacedata(pointnumber, x, y, frameid) VALUES (35, 34, 34, 6)
********************************
INSERT INTO openfacedata(pointnumber, x, y, frameid) VALUES (68, 67, 67, 6)
UPDATE video SET (numframes, framespersecond, width, height) = (3, 30, 640, 480) WHERE videoid = 2
jw@jw-laptop ~/CS160Project/vidproc $ 
#endif

