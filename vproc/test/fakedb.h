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

//#include <postgresql/libpq-fe.h>
//#include <libpq-fe.h>

struct DataBase
{
    bool connect_ok()
    {
        return true;
    }

    bool begin_transaction_ok()
    {
        return true;
    }

    //bool connect_and_begin_transaction_ok(const char* connstr = "user=postgres host=localhost password=postgres dbname=cs160");

    void do_something_with_results(const FrameResults& , unsigned)
    {

    }

    void update_video(const VideoMetadata&, unsigned)
    {

    }

    bool end_transaction_ok()
    {
        return true;
    }

    void disconnect()
    {

    }
};

inline
unsigned long get_millisecs_stamp(void)
{
    struct timespec spec;

    clock_gettime(CLOCK_MONOTONIC, &spec);

    return spec.tv_sec*1000u + spec.tv_nsec/(1000u*1000u);//10e6 millisecs per nanosec
}

inline
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

#endif // H_HEADER

