/*
    This program loads an image into a cv::Mat, detects 68 facial points using dlib,
    and draws a Delauny triangulation on them.

    Run it with
    $progname in_img out_img

    The rob.png file from his draw_delauny example works good

    The shape_predictor_68_face_landmarks.dat file is needed, and can be downloaded online.
    Right now is hardcoded to look in current directory, sorry...

    Can compile this with:
    g++ -Wall -Wextra -std=c++11 -O2 *.cpp -s `pkg-config --libs opencv dlib-1`

    See below for how I installed dlib on Mint:
*/

/*
    Installing dlib, based on: https://www.learnopencv.com/install-dlib-on-ubuntu/

    First, have opencv installed system wide if not already,
    http://www.techgazet.com/install-opencv/

    get release v19.9
    https://github.com/davisking/dlib/releases

    (Optional speedup) go into examples/CMakeLists.txt and comment out some examples on the bottom,
    we probably don't need any. Here are a few that seems most relevant:
    if (false)
        ... original block ...
    else()
        add_gui_example(train_object_detector)
        add_example(train_shape_predictor_ex)
        add_gui_example(object_detector_advanced_ex)
        add_gui_example(object_detector_ex)
        add_gui_example(image_ex)
        add_gui_example(face_detection_ex)
        add_gui_example(face_landmark_detection_ex)
        add_gui_example(fhog_ex)
        add_gui_example(fhog_object_detector_ex)
    endif()

    tar xvf dlib-19.9.tar.gz
    cd dlib-19.9/
    mkdir build
    cd build
    cmake .. -DUSE_AVX_INSTRUCTIONS=1 -DDLIB_PNG_SUPPORT=1 -DDLIB_JPEG_SUPPORT=1
    cmake --build . --config Release
    sudo make install
    sudo ldconfig

    How to compile/link a project using dlib and opencv
    (note some linker flags may be optional, and the order of libraries and .o's matter)

    For release mode, add -O2 -s

    g++ -DDLIB_PNG_SUPPORT=1 -DDLIB_JPEG_SUPPORT=1 -Wall -Wextra -std=c++11 -O2 \
    -s `pkg-config --libs opencv dlib-1` -lpthread -lpng -ljpeg -lX11 *.cpp -o program.out
*/

/*
    Notes:
    Am not looping thru all rects if multiple faces.
    Can perhaps downsample img to pass to get the bounding rect, then mul back. Also use dat from previous frame.

    Have persistent backend server process, so crap dlib init/iostreams done once.

    cv::VideoCap can extract mats from a webcam, maybe other video formats too.

    cv will give yaw pitch and roll cv::solvePnP after giving it a selection of the 68 points.
    https://www.learnopencv.com/head-pose-estimation-using-opencv-and-dlib/

    All 68 points known, so can draw triangles directly, avoiding subdiv
    (see render_face).

    Memory allocations and copys/conversions galore
*/

#include <opencv2/opencv.hpp>

#include <dlib/opencv.h>
#include <dlib/image_processing/frontal_face_detector.h>
#include <dlib/image_processing.h>

#include <stdio.h>

void drawDelaunay68(cv::Mat& im, const dlib::full_object_detection& parts)
{
    #if 1
    cv::Size const dims = im.size();
    cv::Rect const rect = {0,0,dims.width,dims.height};
	cv::Subdiv2D subdiv(rect);
	for (unsigned i=0; i!=68; ++i)
	{
		dlib::point dpt = parts.part(i);
		subdiv.insert({float(dpt.x()), float(dpt.y())});
	}
	std::vector< cv::Vec6f > trilist;
	subdiv.getTriangleList(trilist);

	const cv::Scalar delaunay_color(255,0,0);//bgr

	for (auto const t : trilist)
    {
        const cv::Point pt0(t[0], t[1]);
        const cv::Point pt1(t[2], t[3]);
        const cv::Point pt2(t[4], t[5]);
        //the known 68 can avoid this too
        if (rect.contains(pt0) && rect.contains(pt1) && rect.contains(pt2))
        {
            cv::line(im, pt0, pt1, delaunay_color, 1, CV_AA, 0);
            cv::line(im, pt1, pt2, delaunay_color, 1, CV_AA, 0);
            cv::line(im, pt2, pt0, delaunay_color, 1, CV_AA, 0);
        }
    }
    #else
    for (unsigned i=0; i!=68; ++i)
    {
        dlib::point dpt = parts.part(i);

        cv::circle(im, cv::Point(dpt.x(), dpt.y()), 4, cv::Scalar(0, 0, 255), CV_FILLED, CV_AA, 0);
    }
    #endif
}

#define ERROR(M) do{puts(M); return -1;}while(0)

int main(int argc, const char** argv)
{
	if (argc!=3)
		ERROR("Usage: executable input_image output_image");

    dlib::frontal_face_detector ffacedetector = dlib::get_frontal_face_detector();
    dlib::shape_predictor shapepred;

	try//need to use exceptions to handle this?
	{
		dlib::deserialize("shape_predictor_68_face_landmarks.dat") >> shapepred;
	}
	catch(const dlib::serialization_error& e)
	{
		puts("error loading shape_predictor data");
		ERROR(e.what());
	}

	cv::Mat im = cv::imread(argv[1]);

	if (im.empty())
		ERROR("error loading image");


	//convert to dlib fmt. this isnt supposed to allocate or copy
	dlib::cv_image< dlib::bgr_pixel > const cimg(im);
	//more than one face can be detected, so can loop through
	std::vector<dlib::rectangle> faces = ffacedetector(cimg);//operator()
	if (faces.empty())
	{
		puts("no face detected");
		return -1;
	}
	dlib::full_object_detection const marks = shapepred(cimg, faces[0]);
	if (marks.num_parts()!=68)
		ERROR("num_parts not 68");


	drawDelaunay68(im, marks);
	//render_face(im, marks);

	cv::imwrite(argv[2], im);

	return 0;
}

#if 0
Git workflow guide:
https://www.atlassian.com/git/tutorials/comparing-workflows#centralized-workflow

Postgres cheatsheet:
https://gist.github.com/Kartones/dd3ff5ec5ea238d4c546

Vide processing

Dlib face detection per video image split by cv VideoCap
https://github.com/davisking/dlib/blob/master/examples/webcam_face_pose_ex.cpp

Dlib face det single img:
https://github.com/davisking/dlib/blob/master/examples/face_landmark_detection_ex.cpp

Pose detection using CV solvePnp
https://www.learnopencv.com/head-pose-estimation-using-opencv-and-dlib/
https://github.com/spmallick/learnopencv/blob/master/HeadPose/headPose.cpp
^ is from guy who made delaunay triangle tut
https://github.com/spmallick/dlib/blob/master/examples/webcam_head_pose.cpp
https://github.com/spmallick/dlib/blob/master/examples/render_face.hpp
^ his fork of dlib

If server not in C++, perhaps can communicate with processing part using unix domain sockets?
https://stackoverflow.com/questions/4789720/communicate-c-program-and-php/43421610#43421610
Have to agree on send api. Eg: front end sense file path of video to process and where to send it to when done.


#endif // 0

#if 0
void draw_polyline(cv::Mat &img, const dlib::full_object_detection& d, const int start, const int end, bool isClosed = false)
{
    std::vector <cv::Point> points;
    for (int i = start; i <= end; ++i)
    {
        points.push_back(cv::Point(d.part(i).x(), d.part(i).y()));
    }
    cv::polylines(img, points, isClosed, cv::Scalar(255,0,0), 2, 16);

}

void render_face (cv::Mat &img, const dlib::full_object_detection& d)
{
    /*
    DLIB_CASSERT
    (
     d.num_parts() == 68,
     "\t std::vector<image_window::overlay_line> render_face_detections()"
     << "\n\t Invalid inputs were given to this function. "
     << "\n\t d.num_parts():  " << d.num_parts()
     );
     */

    draw_polyline(img, d, 0, 16);           // Jaw line
    draw_polyline(img, d, 17, 21);          // Left eyebrow
    draw_polyline(img, d, 22, 26);          // Right eyebrow
    draw_polyline(img, d, 27, 30);          // Nose bridge
    draw_polyline(img, d, 30, 35, true);    // Lower nose
    draw_polyline(img, d, 36, 41, true);    // Left eye
    draw_polyline(img, d, 42, 47, true);    // Right Eye
    draw_polyline(img, d, 48, 59, true);    // Outer lip
    draw_polyline(img, d, 60, 67, true);    // Inner lip

}

static cv::Rect dlibRectangleToOpenCV(dlib::rectangle r)
{
    return cv::Rect(cv::Point2i(r.left(), r.top()), cv::Point2i(r.right() + 1, r.bottom() + 1));
}
#endif
