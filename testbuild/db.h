#ifndef H_HEADER
#define H_HEADER

/*
    These are gathered now also; once at the start of the process.

    nframes in th output video is not final until all computation is done.
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

    int16_t x() const {return x16;}//dumb getters, was trying to fit it into dlib
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
//this is a 292 byte structure
static_assert(sizeof(FrameResults)==296, "");

#endif // H_HEADER
