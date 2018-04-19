//For the person doing the database stuff, I put @DB in source code locations in this file and "db.h"
//to point out things of interest.

//See build.txt for guide to build

/*              WHAT THIS DOES:

    Takes 1 command line arg, the video file's (absolute) name.
    Outputs the RELATIVE name prefixed with "out_" to the current directory.
    Say this executable is /home/jw/Data/vprocess.out
    Then:
        jw@jw-laptop ~/Data $ ./vprocess.out /home/jw/a/b/c/Good/contour.avi
    Will produce a file:
        /home/jw/Data/out_contour.avi
*/

/*
    Supports reading avi, mp4, and webm.
    webm cannot be sent back as webm though, and is converted to avi.
    ^right now appends .avi, so become .webm.avi, should replace instead?

    Anyway, when we show to "customer", test with .avi or .mp4

    To be safe, build ffmpeg with all options for supporting more formats,
    and THEN opencv, with all options for supporting more formats.
*/

/*
    VideoCapture and VideoWriter tested on
    Linux Mint 18.3
    OpenCV 3.3, built with the following, after first installing ffmpeg:

    sudo apt-get -y install libopencv-dev build-essential
    cmake git libgtk2.0-dev pkg-config python-dev python-numpy
    libdc1394-22 libdc1394-22-dev libjpeg-dev libpng12-dev libjasper-dev
    libavcodec-dev libavformat-dev libswscale-dev libgstreamer0.10-dev
    libgstreamer-plugins-base0.10-dev libv4l-dev libtbb-dev libqt4-dev libfaac-dev libmp3lame-dev
    libopencore-amrnb-dev libopencore-amrwb-dev libtheora-dev libvorbis-dev libxvidcore-dev x264 v4l-utils unzip

    Followed guide: http://www.techgazet.com/install-opencv/
*/

/*
    Is a C/C++ webserver too hard? Maybe I'll try later.

    Make this a php or python callable library?
    Shared lib?
    Daemon process (so initialization done once per system start),
    and communicate via ipc, like AF_UNIX udp example?

    Possible optimization:
        The bounding face ROI detection is actually much slower than the subsequent
        68 point detection. If the fps is >= 24 or so, could use the same dimensions
        from the previous detection every other time, skipping the call.
*/
#include "precomp.h"
#include "db.h"

#ifndef TRAINED_FILE
    #define TRAINED_FILE "./shape_predictor_68_face_landmarks.dat"
#endif

/*
    Possible ideas, for all these can add extra parameters and state to main(),
    e.g: like a FILE* or PQ/pqxx state stuff.

    1: fwrite (binary) the whole thing, appending to some file.
    2: Print the stuff as text somewhere.
    3: Pass to C/C++ DB functions.
    4: Some kind of interprocess communication/have another language do something.
    5: Other.
*/
//@DB:
//I guess will call this once per loop
void DoSomethingWithResults(const FrameResults& r/*, any other params you need for impl*/)
{
    //...
}
//@DB:
//Only after this function is called so many times and all the processing is done,
//will the final frame count in the output video be determined, and can subsequently be sent.


//TODO? gut and in more familiar fmt
typedef dlib::point lmCoord;//landmark coordinate pair

//The format returned is [Tx, Ty, Tz, Eul_x, Eul_y, Eul_z]
struct PoseEuler
{
    cv::Vec3d trans; //And I just discard this.
    EulerAnglesF32 e;//yaw pitch and roll
};

static
PoseEuler getPoseAndDraw(cv::Mat& im, const lmCoord* marks);

static
void drawDelaunay68(cv::Mat& im, const lmCoord* marks);

template<class T> T Max(T a, T b) { return b>a ? b : a; }
template<class T> T Min(T a, T b) { return b<a ? b : a; }

/*
    These get-Right/Left-Roi functions cut a rectangle using the 68 points
    gotten from dlib already. The max width and height are taken and expanded a little
    to be conservative (My assumption is its worse to be too small, instead of too big).

    68 point locations reference:
    https://stackoverflow.com/questions/42539240/how-to-get-the-length-of-eyes-and-mouth-using-dlib/44483983#44483983
*/

//left as in looking at the points picture
static
cv::Rect getLeftRoi(cv::Size dims, const lmCoord* a)
{
    int const left  = a[36].x();
    int const width = a[39].x() - left;
    int const top   = Min(a[37].y(), a[38].y());
    int const height= Max(a[41].y(), a[40].y()) - top;
    //for good measure clamp to image:
    cv::Rect const roi = cv::Rect
    (
        Max(left - 1, 0),
        Max(top - 1, 0),
        Min(width+2, dims.width),
        Min(height+2, dims.height)
    );
    return roi;
}
/*
    This is same as above, except
    36 -> 42
    39 -> 45

    37 -> 43
    38 -> 44

    41 -> 47
    40 -> 46
*/
static
cv::Rect getRightRoi(cv::Size dims, const lmCoord* a)
{
    int const left  = a[42].x();
    int const width = a[45].x() - left;
    int const top   = Min(a[43].y(), a[44].y());
    int const height= Max(a[47].y(), a[46].y()) - top;
    //for good measure clamp to image:
    cv::Rect const roi = cv::Rect
    (
        Max(left - 1, 0),
        Max(top - 1, 0),
        Min(width+2, dims.width),
        Min(height+2, dims.height)
    );
    return roi;
}

extern//in hume.cpp
cv::Point findEyeCenter(const cv::Mat& im, cv::Rect eye);

static
cv::Point detectPupilHume(cv::Mat& im, cv::Rect eye)
{
    using namespace cv;

    Mat wholeGray;//he was doing some weird split channels thing that didn't work for me?
    cvtColor(static_cast<const Mat&>(im), wholeGray, CV_BGR2GRAY);

    cv::Point const pnt = findEyeCenter(static_cast<const Mat&>(wholeGray), eye);//rel to eye rect

    cv::Point const icare = cv::Point(pnt.x + eye.x, pnt.y + eye.y);
    return icare;
}

int main(int argc, char **argv)
{
    if (argc!=2)
    {
        fprintf(stderr, "usage: %s <video-file>\n", argv[0]);
        return -1;
    }
    const char *const vfilename = argv[1];
    cv::VideoCapture vcap(vfilename);
    if (!vcap.isOpened())
    {
        fprintf(stderr, "failed to capture video [%s]\n", vfilename);
        return -1;
    }

    //Assignment 3 req: get video data.
    VideoMetadata vdata;
    vdata.nframes = vcap.get(CV_CAP_PROP_FRAME_COUNT);//@DB: note this is not sent at the beginning
    vdata.fps     = vcap.get(CV_CAP_PROP_FPS);
    vdata.width   = vcap.get(CV_CAP_PROP_FRAME_WIDTH);
    vdata.height  = vcap.get(CV_CAP_PROP_FRAME_HEIGHT);
    int final_nframes = vdata.nframes;
    //fprintf(stderr, "%lf\n", vcap.get(CV_CAP_PROP_FOURCC));
    //const int bDumpData = ! isatty(1);

    const char * slash = strrchr(vfilename, '/');//reverse
    cv::String voutname("out_");
    voutname += (slash==nullptr ? vfilename : slash+1);

    cv::VideoWriter vwriter(voutname,
                            vcap.get(CV_CAP_PROP_FOURCC),
                            vdata.fps,
                            cv::Size(vdata.width, vdata.height));
    if (!vwriter.isOpened())
    {
        //trying this is better than nothing
        fprintf(stderr, "failed create video writer in uploaded format,\nattempting to convert to avi\n");
        //int dotpos = (int) voutname.find_last_of('.');
        //if (dotpos >= 0) voutname.resize(dotpos);
        voutname += ".avi";
        //so if it ends in .webm, will now end in .webm.avi. perhaps change. Anyway, when we show to "customer", test with .avi or .mp4
        if (!vwriter.open(  voutname,
                            877677894.0,//vcap.get(cv::VideoWriter::fourcc()),
                            vdata.fps,
                            cv::Size(vdata.width, vdata.height)))
        {
            fputs("could not convert video for writer, exiting\n", stderr);
            return -1;
        }
    }

    fprintf(stderr, "%s\n", voutname.c_str());

    dlFaceRoiFinder faceRectFinder;
    dlFaceMarkDetector markDetector;

    if (markDetector.init(TRAINED_FILE)!=0)//I made it print too...
        return -1;

    cv::Mat im;
    FrameResults all={};

    fputs("entering read/process loop\n", stderr);

    for (int i=0; i < final_nframes; ++i)
    {
        all.frameno = i;//@DB: zero-based

        if (!vcap.read(im))
        {
            //if this happens (unlikely if VideoCap opened successfully),
            //smush the video
            //@DB this is where final frames in output may not match input
            fprintf(stderr, "failed to extract frame %d\n", i);
            --final_nframes;
            --i;//then gets inc'd back to same in continue. reuse
            continue;
        }

        //convert to dlib fmt. this isnt supposed to allocate or copy
        dlib::cv_image< dlib::bgr_pixel > const cimg(im);
        //dlib::cv_image<unsigned char> const cimg(im);
        //more than one face can be detected, so can loop through
        std::vector<dlib::rectangle> const faces = faceRectFinder.findFaceRects(cimg);
        dlFaceMarkResults detRes;

        if (faces.empty() || (detRes = markDetector.detectMarks(cimg, faces[0]), detRes.num_parts()) != 68){
            fprintf(stderr, faces.empty() ? "failed to get face ROI for frame %u\n":"failed to detect 68 marks for frame %u\n", i);
            all.marks68[0] = {-1,-1};//signal error
        }
        else
        {
            const lmCoord *const marks = &detRes.part(0);
            auto const cnvt=[](dlib::point p){ return PairInt16{int16_t(p.x()), int16_t(p.y())}; };
            //1: store marks
            for (int i=0; i!=68; ++i)
                all.marks68[i] = cnvt(marks[i]);

            cv::Size const imdims =  im.size();

            auto leyeRect = getLeftRoi(imdims, marks);
            cv::Point leftEyeCoord = detectPupilHume(im, leyeRect);

            auto reyeRect = getRightRoi(imdims, marks);
            cv::Point rightEyeCoord = detectPupilHume(im, reyeRect);

            drawDelaunay68(im, marks);//do after pupil detection
            all.rotation = getPoseAndDraw(im, marks).e;//2: store Euler angles. discard translation...

            auto const valid=[](cv::Point p, cv::Size dim){
                return p.x>=0 && p.x<dim.width && p.y>=0 && p.x<dim.height;
            };//end lambda
            //3: store pupils
            //I was drawing them before. If the video is good quality,
            //the persons face is close to the camera and not moving much then it looks good.
            //If not, its too imprecise and is detracting.
            //Regardless, the FrameResults get filled, and that gets sent to where it needs to go.
            if (valid(leftEyeCoord, imdims)) {
                cv::circle(im, leftEyeCoord, 3, CV_RGB(255, 255, 0), CV_FILLED, CV_AA, 0);
                all.left_pupil = {int16_t(leftEyeCoord.x), int16_t(leftEyeCoord.y)};
            } else
                all.left_pupil = {-1,-1};
            //same as above, but for right
            if (valid(rightEyeCoord, imdims)) {
                cv::circle(im, rightEyeCoord, 3, CV_RGB(255, 255, 0), CV_FILLED, CV_AA, 0);
                all.right_pupil = {int16_t(rightEyeCoord.x), int16_t(rightEyeCoord.y)};
            } else
                all.right_pupil = {-1,-1};
        }//end if have 68 points

        vwriter.write(im);//the return type of this is void
        DoSomethingWithResults(all);//@DB
    }//for each frame

    //if (!vwriter.release()) {}//returns void...
    vwriter.release();

    //@DB: The video metadata can  be sent now
    vdata.nframes = final_nframes;

    return 0;
}
//location of all 68 points are known relative to each other, could draw directly
void drawDelaunay68(cv::Mat& im, const lmCoord* lndmks)
{
    cv::Size const dims = im.size();
    cv::Rect const rect = {0,0,dims.width,dims.height};
	cv::Subdiv2D subdiv(rect);
	for (unsigned i=0; i!=68; ++i)
	{
		lmCoord const dpt = lndmks[i];
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
    //do this after so points are on top of triangle edges, I think looks nicer
    for (unsigned i=0; i!=68; ++i)
	{
		lmCoord const dpt = lndmks[i];//TODO, change radius based on dims
		cv::circle(im, cv::Point(dpt.x(), dpt.y()), 2, cv::Scalar(0, 0, 255), CV_FILLED, CV_AA, 0);
	}
}

inline cv::Point2d lmc2cv2d(const lmCoord v)
{
    return cv::Point2d(v.x(), v.y());
}

static
EulerAnglesF32 AxisAngle2Euler(const cv::Vec3d& axis_angle);

static
PoseEuler getPoseAndDraw(cv::Mat& im, const lmCoord* marks)
{
    using namespace cv;

    const std::array<Point2d, 6> image_points =
    {{
        lmc2cv2d(marks[30]),//Nose tip
        lmc2cv2d(marks[ 8]),//Bottom of Chin
        lmc2cv2d(marks[36]),//Left eye left corner
        lmc2cv2d(marks[45]),//Right eye right corner
        lmc2cv2d(marks[48]),//Right mouth corner
        lmc2cv2d(marks[54])
    }};

    for (Point2d const point : image_points)
        circle(im, point, 2, cv::Scalar(0, 255, 0), CV_FILLED, CV_AA, 0);

    static
    const std::array<Point3d, 6> model_points =
    {{
        Point3d(0.0f, 0.0f, 0.0f),               // Nose tip
        Point3d(0.0f, -330.0f, -65.0f),          // Chin
        Point3d(-225.0f, 170.0f, -135.0f),       // Left eye left corner
        Point3d(225.0f, 170.0f, -135.0f),        // Right eye right corner
        Point3d(-150.0f, -150.0f, -125.0f),      // Left Mouth corner
        Point3d(150.0f, -150.0f, -125.0f)        // Right mouth corner
    }};

    // Camera internals
    double focal_length = im.cols; // Approximate focal length.
    Point2d center = cv::Point2d(im.cols/2,im.rows/2);
    cv::Mat camera_matrix = (cv::Mat_<double>(3,3) << focal_length, 0, center.x, 0 , focal_length, center.y, 0, 0, 1);
    cv::Mat dist_coeffs = cv::Mat::zeros(4,1,cv::DataType<double>::type); // Assuming no lens distortion
    // Output rotation and translation
    //cv::Mat translation_vector, rotation_vector; // Rotation in axis-angle form
    cv::Vec3d translation_vector, rotation_vector; // Rotation in axis-angle form


    // Solve for pose
    cv::solvePnP(model_points, image_points, camera_matrix, dist_coeffs, rotation_vector, translation_vector);
    // Project a 3D point (0, 0, 1000.0) onto the image plane.
    // We use this to draw a line sticking out of the nose

    static
    const std::array<Point3d, 1> nose_end_point3D = {{Point3d(0,0,1000.0)}};

    std::vector<Point2d> nose_end_point2D;

    projectPoints(nose_end_point3D, rotation_vector, translation_vector, camera_matrix, dist_coeffs, nose_end_point2D);

    cv::line(im, image_points[0], nose_end_point2D[0], cv::Scalar(0,255,0), 2);

    return PoseEuler{translation_vector, AxisAngle2Euler(rotation_vector)};
}

//This is adapted from
//https://www.learnopencv.com/rotation-matrix-to-euler-angles/
static
EulerAnglesF32 rotationMatrixToEulerAngles(const cv::Matx33d& R)
{
    double sy = sqrt(R(0,0)*R(0,0) +  R(1,0)*R(1,0));

    bool singular = sy < 1e-6;

    if (!singular)
        return EulerAnglesF32
        {
            atan2f(R(2,1) , R(2,2)),
            atan2f(-R(2,0), sy),
            atan2f(R(1,0), R(0,0))
        };
    else
        return EulerAnglesF32
        {
            atan2f(-R(1,2), R(1,1)),
            atan2f(-R(2,0), sy),
            0.0f
        };
}

static
EulerAnglesF32 AxisAngle2Euler(const cv::Vec3d& axis_angle)
{
    cv::Matx33d rotation_matrix;
    cv::Rodrigues(axis_angle, rotation_matrix);

    return rotationMatrixToEulerAngles(rotation_matrix);//learn opencv version
}

    //This comment applies to streaming it to a pipe,
    //because cant frewind on a pipe(). Output video may not have same number of frames as input video.
    //
    //Could try shared mem.

    //NOTE: when written, this is the MAX frame count,
    //some frames may be lost.
    //use the max frame count to allocate a suitable buffer,
    //then read as much as you can,
    //and use the bytes written to determine the number of frames
    //in the final video.
