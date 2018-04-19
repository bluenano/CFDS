#ifndef H_PCH
#define H_PCH

//should I just include <opencv2/opencv.hpp>

#include <opencv2/core/mat.hpp>
#include <opencv2/videoio.hpp>//VideoCapture could be useful, and this prob gets pulled in someway or another
#include <opencv2/imgproc.hpp>//SubDiv2D
#include <opencv2/imgcodecs.hpp>//im read/write
#include <opencv2/highgui/highgui.hpp>//imshow
#include <opencv2/calib3d.hpp>//solvePnP, project points

//dlib::point is incredibly over-engineered and includes the entire world... but Its not worth dancing around the return types

#include <dlib/opencv.h>
#include <dlib/image_processing/full_object_detection.h>

#include <stdio.h>
/*
rectangle rect;
std::vector<point> parts;
*/

class dlFaceRoiFinder
{
    alignas(16) unsigned char m[256];
public:
    dlFaceRoiFinder();//def ctor
    ~dlFaceRoiFinder();//destructor
    std::vector<dlib::rectangle> findFaceRects(const dlib::cv_image<dlib::bgr_pixel>&);
};

typedef dlib::full_object_detection dlFaceMarkResults;

class dlFaceMarkDetector
{
    alignas(16) unsigned char m[128];
public:
    dlFaceMarkDetector();//def ctor
    ~dlFaceMarkDetector();//destructor
    int init(const char *);//success if 0
    dlFaceMarkResults detectMarks(const dlib::cv_image<dlib::bgr_pixel>&, const dlib::rectangle&);
};
#endif // H_PCH

