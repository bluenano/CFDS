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

template<int I>
class DLibFaceDetectorPyDown
{
    alignas(16) unsigned char m[256];
public:
    DLibFaceDetectorPyDown();
    ~DLibFaceDetectorPyDown();
    std::vector<dlib::rectangle> findFaceRects(const dlib::cv_image<dlib::bgr_pixel>&);//THIS IS NOT CONST, CARE MULTITHREAD
};

//lets experiment with different params
//why are these templates, and not something can pass at run time?
//maybe I should do a noop one, and do the scaling manually, so not 18 million templates...
//but after testing, will prob get rid of all but 1.
//extern template class DLibFaceDetectorPyDown<6>;//the regular one
extern template class DLibFaceDetectorPyDown<4>;
//extern template class DLibFaceDetectorPyDown<2>;

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

