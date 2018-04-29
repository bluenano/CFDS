#ifndef H_PCH
#define H_PCH

#include <opencv2/core/mat.hpp>
#include <opencv2/videoio.hpp>//VideoCapture, VideoWriter
#include <opencv2/imgproc.hpp>//SubDiv2D
#include <opencv2/imgcodecs.hpp>//im[read|write]
#include <opencv2/highgui/highgui.hpp>//imshow
#include <opencv2/calib3d.hpp>//solvePnP, projectPoints

//Hard to get away with not including these, because of intermediate dlib::cv_image
#include <dlib/opencv.h>
#include <dlib/image_processing/full_object_detection.h>

#include <stdint.h>
#include <stdio.h>

#include <pthread.h>

#ifdef WANT_6_5
class DLibFaceDetector
{
    alignas(16) unsigned char m[256];
public:
    DLibFaceDetector();//def ctor
    ~DLibFaceDetector();//destructor
    std::vector<dlib::rectangle> findFaceRects(const dlib::cv_image<dlib::bgr_pixel>&);//THIS IS NOT CONST, CARE MULTITHREAD
};
#endif

class DLibFaceDetector_2_1
{
    alignas(16) unsigned char m[256];
public:
    DLibFaceDetector_2_1();//def ctor
    ~DLibFaceDetector_2_1();//destructor
    std::vector<dlib::rectangle> findFaceRects(const dlib::cv_image<dlib::bgr_pixel>&);//THIS IS NOT CONST, CARE MULTITHREAD
};

class DLibLandMarkDetector
{
    alignas(16) unsigned char m[128];
public:
    DLibLandMarkDetector();//def ctor
    ~DLibLandMarkDetector();//destructor
    int init(const char *);//success if 0
    dlib::full_object_detection detectMarks(const dlib::cv_image<dlib::bgr_pixel>&, const dlib::rectangle&) const;
};

#endif // H_PCH

