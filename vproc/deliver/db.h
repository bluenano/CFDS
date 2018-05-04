/*
    Is printing to stdout in apache/cgi a problem?

    For whoever does the database code,
    I marked areas with @DB for things to note.
*/
#ifndef H_HEADER
#define H_HEADER

#include <stdint.h>

/*
    These are gathered now also; once at the start of the process.

    nframes in the output video is not final until all computation is done.
*/
struct VideoMetadata
{
    int32_t nframes;
    int32_t fps;
    int32_t width;
    int32_t height;
};
//16 byte header
static_assert(sizeof(VideoMetadata)==16, "");

struct alignas(4) PairInt16
{
    int16_t x16, y16;//2 bytes each

    int16_t x() const {return x16;}//dumb getters
    int16_t y() const {return y16;}
};


struct EulerAnglesF32
{
    float yaw, pitch, roll;//4 bytes each
};

/*
    @DB:
    One of these is filled in each *SUCCESFULLY read and written* frame.
    If read but no detection, will unmodified image with a "blank" result to ensure random access consistency.
    In that case, some fields will set to error conditions or
    unfilled/stale data.

    @DB:
    Also, please note that it is possible that the output video will have less frames than the input due to
    decoding/encoding read/write errors.
    Consult with the VideoMeta struct, which can only be finalized after all processing,
    and thus can only be sent after then.
*/
struct FrameResults//a POD struct
{
    uint32_t frameno;//this might not need to be here since instances of this obj are stored as in an array,
                     //but it was requested.

    PairInt16 marks68[68];  //if marks68[0].x==-1 then there was no detection,
                            //and nothing else is filled in for this entire object

    PairInt16 left_pupil;   //if .x==-1 then not found
    PairInt16 right_pupil;  //if .x==-1 then not found

    EulerAnglesF32 rotation;//if marks68 found, then these will be filled in
};
//this is a 296 byte structure
static_assert(sizeof(FrameResults)==296, "");

#include <postgresql/libpq-fe.h>

class DataBase
{
    PGconn *conn=nullptr;
    bool error_occurred = false;

    bool do_print_insert_frame = true;
    bool do_print_insert_landmarks = true;

    char *fsuffix;
    char *lmsuffix;

    char fbuf[1024];
    char lmbuf[1024];

    int insert_frame_returning_frameid(const FrameResults& frame, unsigned videoid);

    void insert_landmarks(const FrameResults& frame, unsigned frameid);

    void set_error_and_rollback(); // have to do this if want to execute other statements after wards, for daemon.
public:
    //build silly strings or prepare statements in the constructor
    DataBase();

    bool connect_ok()
    {
        this->conn = PQconnectdb("user=postgres host=localhost password=postgres dbname=cs160");

        if (PQstatus(conn) != CONNECTION_OK)
        {
            fprintf(stderr, "Connection to database failed:\nPQ: %s", PQerrorMessage(conn));
            this->error_occurred = true;
        }

        return true;
    }

    bool begin_transaction_ok()
    {
        //if (error_occurred)//If daemon, should try reset on subsequent?
            //return false;
        //yeah, will do that. shouldnt get here if conn failed anyway
        error_occurred = false;

        PGresult *const res = PQexec(conn, "BEGIN");

        if (PQresultStatus(res) != PGRES_COMMAND_OK)
        {
            fprintf(stderr, "BEGIN command failed:\nPQ: %s", PQresultErrorMessage(res));
            this->error_occurred = true;

            return false;
        }
        else
            return true;
    }

    //bool connect_and_begin_transaction_ok(const char* connstr = "user=postgres host=localhost password=postgres dbname=cs160");

    void do_something_with_results(const FrameResults& all, unsigned videoid);

    void update_video(const VideoMetadata& vmeta, unsigned);

    bool end_transaction_ok();

    void disconnect()
    {
        if (conn)
        {
            PQfinish(conn);
            conn=nullptr;
        }
    }

    ~DataBase() { disconnect(); }
};

unsigned long get_millisecs_stamp(void);
void millisleep(unsigned long msecs);

#endif // H_HEADER
