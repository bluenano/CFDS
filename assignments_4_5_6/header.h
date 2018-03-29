#ifndef H_HEADER
#define H_HEADER

//typedef dlib::point lmCoord;//landmark coordinate pair

//typedef lmCoord image_coords_xy;

struct alignas(4) PairInt16
{
    int16_t x16, y16;//2 bytes each

    int16_t x() const {return x16;}
    int16_t y() const {return y16;}
};

//typedef dlib::point lmCoord;//landmark coordinate pair

//typedef xy_coords_int16 lmCoord;

struct EulerAnglesF32
{
    float yaw, pitch, roll;//4 bytes each
};

//note error conditions
struct FrameResults//a POD struct
{
    PairInt16 marks68[68];//if marks68[0].x==-1 then was an error, known where found (no pupils either)

    PairInt16 left_pupil;//if .x==-1 not found
    PairInt16 right_pupil;//if .x==-1 not found
    //70 * 4
    EulerAnglesF32 rot;//+ 4*3 //if marks found, always found
};

static_assert(sizeof(FrameResults)==292, "");

#endif // H_HEADER
