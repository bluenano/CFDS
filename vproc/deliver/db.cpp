//gonna stuff these timer definitions here

#ifndef _POSIX_C_SOURCE
    #define _POSIX_C_SOURCE 200809L
#elif (_POSIX_C_SOURCE) < 199309L
    #error compile with -D_POSIX_C_SOURCE=200809L
#endif

#include <time.h>
extern
unsigned long get_millisecs_stamp(void)
{
    struct timespec spec;

    clock_gettime(CLOCK_MONOTONIC, &spec);

    return spec.tv_sec*1000u + spec.tv_nsec/(1000u*1000u);//10e6 millisecs per nanosec
}

extern
void millisleep(unsigned long msecs)
{
    struct timespec a[2];//req, rem

    if (msecs >= 1000u)
    {
        a[0].tv_sec = msecs/1000u;
        a[0].tv_nsec = msecs*1000u;//only by one 1000 because secs consumed
    }
    else
    {
        a[0].tv_sec = 0;
        a[0].tv_nsec = msecs*(1000u*1000u);
    }
    #if 0//untested
    unsigned i=0;
    while (nanosleep(&a[i], &a[i^1])!=0 && errno==EINTR)
        i^=1;
    #else
    nanosleep(a+0, a+1);//const request, remaining (due to thread signal/intr) not a concern for this assignment
    #endif
}

#include "db.h"

#include <stdio.h>
#include <stdlib.h>

//not sure if should put everything in a transaction,
//or delete all previous inserts table if something fails midway.

/*
class ScopePGResult
{
    PGresult *res;
public:
};
*/

static
int mycopy(char* dest, const char* src)
{
    int i = 0;
    char ch;
    for ( ; (ch = src[i]) != '\0'; ++i)
        dest[i] = ch;
    return i;
}

//build silly strings or prepare statements in the constructor
/*ctor*/ DataBase::DataBase()
{
    int i = mycopy(fbuf,
        "INSERT INTO frame("
        "videoid, "
        "framenumber, "
        "ftpupilrightx, "
        "ftpupilrighty, "
        "ftpupilleftx, "
        "ftpupillefty, "
        "roll, "//not ordered yaw, pitch, roll in schema?
        "pitch, "//give explicit value names
        "yaw)"
        "VALUES (");
    this->fsuffix = fbuf + i;

    i = mycopy(lmbuf, "INSERT INTO openfacedata(pointnumber, x, y, frameid) VALUES (");
    this->lmsuffix = lmbuf + i;
}

static
bool res_ok(const PGresult* r)
{
    ExecStatusType const v = PQresultStatus(r);
    return v==PGRES_COMMAND_OK || v==PGRES_TUPLES_OK;//which one in case of RETURNING?
}

#if 0
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
#endif // 0

void DataBase::set_error_and_rollback()
{
    this->error_occurred = true;

    PGresult *const res = PQexec(conn, "ROLLBACK");

    if (PQresultStatus(res) != PGRES_COMMAND_OK)
        fprintf(stderr, "ROLLBACK command failed:\nPQ: %s", PQresultErrorMessage(res));

    PQclear(res);
}

#if 0
//bool DataBase::connect_and_begin_transaction_ok(const char* connstr = "user=postgres host=localhost dbname=cs160")
bool DataBase::connect_and_begin_transaction_ok(const char* connstr)
{
    bool bret = false;

    this->conn = PQconnectdb(connstr);

    if (PQstatus(conn) == CONNECTION_OK)
    {
        PGresult *const res = PQexec(conn, "BEGIN");
        if (PQresultStatus(res) == PGRES_COMMAND_OK)
        {
            bret = true;
        }
        else
        {
            fprintf(stderr, "BEGIN command failed:\nPQ: %s", PQresultErrorMessage(res));
            this->error_occurred = true;
        }

        PQclear(res);
    }
    else
    {
        fprintf(stderr, "Connection to database failed:\nPQ: %s", PQerrorMessage(conn));
        this->error_occurred = true;
    }

    return bret;
}
#endif

bool DataBase::end_transaction_ok()
{
    if (error_occurred)
        return false;

    PGresult *const res = PQexec(conn, "END");
    bool bret=true;

    if (PQresultStatus(res) != PGRES_COMMAND_OK)
    {
        fprintf(stderr, "End command failed:\nPQ: %s", PQresultErrorMessage(res));
        //error_occurred = true;
        set_error_and_rollback();
        bret = false;
    }

    PQclear(res);
    return bret;
}

//this gets called for every frame... should look into prepared statements
int DataBase::insert_frame_returning_frameid(const FrameResults& all, unsigned videoid)
{
    if (error_occurred)
        return -1;

    #if 0
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
        "VALUES (%d, %d, %d, %d, %d, %u, %f, %f, %f) "
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
    #else
    sprintf(fsuffix, "%d, %d, %d, %d, %d, %d, %f, %f, %f) RETURNING frameid",//NO UNSIGNED
        videoid,
        all.frameno,
        all.right_pupil.x16,
        all.right_pupil.y16,
        all.left_pupil.x16,
        all.left_pupil.y16,
        all.rotation.roll,
        all.rotation.pitch,
        all.rotation.yaw);
    char *buf = this->fbuf;
    #endif

    if (do_print_insert_frame)
        puts(buf), do_print_insert_frame = false;

    PGresult *const res = PQexec(conn, buf);
    long retval = -1;

    if (res_ok(res))
    {
        retval = atoi(PQgetvalue(res, 0, 0));
    }
    else
    {
        fprintf(stderr, "insert frame command failed:\nPQ: %s", PQresultErrorMessage(res));
        //this->error_occurred = true;
        set_error_and_rollback();
    }

    PQclear(res);
    return retval;
}

void DataBase::insert_landmarks(const FrameResults& all, unsigned frameid)
{
    if (error_occurred)
        return;
    #if 0
    char buf[1024];

    static
    const char prefix[] = "INSERT INTO openfacedata(pointnumber, x, y, frameid) VALUES (";

    //copy while getting ptr to end
    int iend = 0;
    for ( ; prefix[iend]!='\0'; ++iend)
        buf[iend] = prefix[iend];
    char *const suffix = buf + iend;
    #else
    char *buf = this->lmbuf;
    #endif

    //It seems pointnumber is not a key and could start at anything,
    //but I'll make it start at 1 for consistency as postgres seems to like that.
    for (int i=0; i<68; ++i)
    {
        sprintf(
            lmsuffix,//suffix,
            "%d, %d, %d, %d)",//no semicolon
            i+1,
            all.marks68[i].x16,
            all.marks68[i].y16,
            frameid);

        if (do_print_insert_landmarks)
            puts(buf), do_print_insert_landmarks = false;

        PGresult *res = PQexec(conn, buf);
        if (PQresultStatus(res) != PGRES_COMMAND_OK)
        {
            fprintf(stderr, "command failed:\nPQ: %s", PQresultErrorMessage(res));
            //error_occurred = true;
            set_error_and_rollback();
        }
        PQclear(res);//after every exec
    }
}

void DataBase::do_something_with_results(const FrameResults& all, unsigned videoid)
{
    unsigned fid = insert_frame_returning_frameid(all, videoid);
    insert_landmarks(all, fid);
}

void DataBase::update_video(const VideoMetadata& vdata, unsigned videoid)
{
    if (error_occurred)
        return;

    char buf[1024];
    sprintf(
        buf,
        "UPDATE video SET (numframes, framespersecond, width, height) = (%u, %u, %u, %u) WHERE videoid = %u",
        vdata.nframes, vdata.fps, vdata.width, vdata.height, videoid);
    puts(buf);

    PGresult *res = PQexec(conn, buf);
    if (PQresultStatus(res) != PGRES_COMMAND_OK)
    {
        fprintf(stderr, "updating videoid %u row command failed:\nPQ: %s", videoid, PQresultErrorMessage(res));
        //error_occurred = true;
        set_error_and_rollback();
    }
    PQclear(res);//after every exec
}
