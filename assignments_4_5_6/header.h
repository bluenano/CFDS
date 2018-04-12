#ifndef H_HEADER
#define H_HEADER

/*
    These are gathered now also; once at the start of the process.

    Again... not sure what do do with them...
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
    One of these is filled in each frame.

    Some fields will be set to error conditions or
    unfilled/stale data if detection failed, see below.
*/
struct FrameResults//a POD struct
{
    PairInt16 marks68[68];  //if marks68[0].x==-1 then there was no detection,
                            //and nothing else is filled in for this entire object

    PairInt16 left_pupil;   //if .x==-1 then not found
    PairInt16 right_pupil;  //if .x==-1 then not found

    EulerAnglesF32 rotation;//if marks68 found, then these will be filled in
};
//this is a 292 byte structure
static_assert(sizeof(FrameResults)==292, "");

//Note there is no frame number, current thoughts are to store as array,
//so the index is the frame offset.
//But if it makes sense to do something else you can add a frame number int field,
//and in fill it in during a loop iteration.

#endif // H_HEADER
