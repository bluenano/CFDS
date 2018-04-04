/*
    XXX:

    By left pupil, do we mean the persons in the video's actual left?
    Right know it is done the other way.
*/
#include "../library/pch.h"

//Need to agree on a place to stuff this:
#ifndef TRAINED_DATA_FILE
    #define TRAINED_DATA_FILE "/home/jw/Data/shape_predictor_68_face_landmarks.dat"
#endif
/*
see readme.txt in library for how to build

command line this program is expecting:
exe_name file_prefix [nframes]

example:
    ./a4.out /home/jw/0 3
The above says the following files exist:
    /home/jw/0.1.png
    /home/jw/0.2.png
    /home/jw/0.3.png

files are expected to follow a format as if made by
    sprintf(buf, "%u.%u.png", file_prefix, frame_no);
or
    ffmpeg -i 0.mp4 0.%d.png      # 0 is video id

Actually, this should work even if the prefix file does not end in a numeric string.

Program expects frame numbers to start at 1, instead of usual 0 b/c it seems thats what ffmpeg wants
to do, and will try to process all files with a frame no <= nframes. If nframes not specified,
will keep going until a file open error.

Also, I think its a good idea to signal how many frames were processed,
right now I do this by process return value. Since the convention is only 0 is success,
nframes - processed is returned. This is meaningless for the optional nframes cmd arg.

Not all usage errors are checked for, as it is assumed process starter will
launch with correct arguments.

This may not be the best way to do things, but I tried to follow the assignment specs,
and serves as reference material for trying something else.

Where the 68 points are:
https://stackoverflow.com/questions/42539240/how-to-get-the-length-of-eyes-and-mouth-using-dlib/44483983#44483983
*/

#include "header.h"
#include <unistd.h>//execl...

static//STUB
void DoSomethingWithResults(const FrameResults& r)
{
    (void)r;
    //TODO... can you help me out with this?

    /*
        Possible ideas, for all these can add extra parameters and state to main(),
        e.g: like a FILE* or PQ/pqxx state stuff.

        1: fwrite (binary) the whole thing, appending to some file.
        2: Print the stuff as text somewhere.
        3: Pass to C/C++ DB functions.
        4: Some kind of interprocess communication/have another language do something.
        5: Other.

        ... what do you think?

        Or since a whole video is made, do we even have to do anything with this?
    */
}

//TODO? gut and in more familiar fmt
typedef dlib::point lmCoord;//landmark coordinate pair

//The format returned is [Tx, Ty, Tz, Eul_x, Eul_y, Eul_z]
struct PoseEuler
{
    cv::Vec3d trans;
    EulerAnglesF32 e;
};

static
PoseEuler getPoseAndDraw(cv::Mat& im, const lmCoord* marks);

#define ERROR(M) do{puts(M); return -1;}while(0)
static void drawDelaunay68(cv::Mat& im, const lmCoord* marks);

template<class T> T Max(T a, T b) { return b>a ? b : a; }
template<class T> T Min(T a, T b) { return b<a ? b : a; }

//left as in looking at the points picture
static
cv::Rect getLeftRoi(cv::Size dims, const lmCoord* a)
{
    int const left  = a[36].x();
    int const width = a[39].x() - left;
    int const top   = Min(a[37].y(), a[38].y());
    int const height= Max(a[41].y(), a[40].y()) - top;
    //LOOK ROB AT ALL THIS EDGE CASE HANDLING
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
    //LOOK ROB AT ALL THIS EDGE CASE HANDLING
    cv::Rect const roi = cv::Rect
    (
        Max(left - 1, 0),
        Max(top - 1, 0),
        Min(width+2, dims.width),
        Min(height+2, dims.height)
    );
    return roi;
}

extern//Mr Hume
cv::Point findEyeCenter(const cv::Mat& im, cv::Rect eye);

static
cv::Point detectPupilHume(cv::Mat& im, cv::Rect eye)
{
    using namespace cv;

    Mat wholeGray;
    cvtColor(static_cast<const Mat&>(im), wholeGray, CV_BGR2GRAY);

    cv::Point const pnt = findEyeCenter(static_cast<const Mat&>(wholeGray), eye);//rel to eye rect

    cv::Point const icare = cv::Point(pnt.x + eye.x, pnt.y + eye.y);
    return icare;
}

int main(int argc, char **argv)
{
    if (argc<2 || argc>=4)
    {
        fprintf(stderr, "usage: %s <file_prefix> [nframes]\n", argv[0]);
        return -1;
    }

    unsigned const nframes = argc==3 ? atoi(argv[2]) : 1u<<16;

    if ((nframes-1u) >= (1u<<16))
        return -1;

    enum{BufCap=512, BufStop=BufCap-65};//for many digits and 0 term shenanigans
    static char buf[BufCap];
    char *pmsd;//ptr to most significant(leftmost) base 10 digit of frame number
    {
        const char *const pfx = argv[1];
        unsigned i=0u;
        for ( ; i!=BufStop && pfx[i]!='\0'; ++i)
            buf[i] = pfx[i];
        if (i==BufStop)
            return -1;
        pmsd = buf+i;
    }
    *pmsd++ = '.';

    dlFaceRoiFinder faceRectFinder;
    dlFaceMarkDetector markDetector;

    if (markDetector.init(TRAINED_DATA_FILE)!=0)//I made it print too...
        return -1;

    FrameResults all;

    unsigned i=1u;
    for (; i<=nframes; ++i)//note 1-based
    {
        sprintf(pmsd, "%u.png", i);//quick and dirty
        cv::Mat im = cv::imread(buf);//this return style means it allocates every time...

        if (im.empty())
            ERROR("error loading image");

        //convert to dlib fmt. this isnt supposed to allocate or copy
        dlib::cv_image< dlib::bgr_pixel > const cimg(im);
        //more than one face can be detected, so can loop through
        std::vector<dlib::rectangle> const faces = faceRectFinder.findFaceRects(cimg);
        dlFaceMarkResults detRes;

        if (faces.empty() || (detRes = markDetector.detectMarks(cimg, faces[0]), detRes.num_parts())!=68)
            //ERROR("no face detected");
            all.marks68[0] = {-1,-1};//signal error
        else
        {
            const lmCoord *const marks = &detRes.part(0);
            auto const cnvt=[](dlib::point p){ return PairInt16{int16_t(p.x()), int16_t(p.y())}; };
            for (int i=0; i!=68; ++i)
                all.marks68[i] = cnvt(marks[i]);//1: store marks
            //My thinking was to turn this into a familiar pod struct (can see in header.h) format instead of
            //dlib crap, so teammates don't have to deal with extra cognitive load trying to figure out what to do with data.
            //
            //This is a peanuts memcpy compared to everything else going on here in this project.
            //If I have time, I may gut dlib shape_predictor to take a PairInt16(&)[68] param
            //and fiddle with the tail of the function.

            cv::Size const imdims =  im.size();

            auto leyeRect = getLeftRoi(imdims, marks);
            cv::Point leftEyeCoord = detectPupilHume(im, leyeRect);

            auto reyeRect = getRightRoi(imdims, marks);
            cv::Point rightEyeCoord = detectPupilHume(im, reyeRect);

            drawDelaunay68(im, marks);//do after pupil detection
            all.rot = getPoseAndDraw(im, marks).e;//2: store Euler angles. discard translation...

            auto const valid=[](cv::Point p, cv::Size dim){
                return p.x>=0 && p.x<dim.width && p.y>=0 && p.x<dim.height;
            };//lambda
            //3: store pupils
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
            #if 1
            pmsd[-1] = 'z';//temp overwrite '.' so make new img
            cv::imwrite(buf, im);
            pmsd[-1] = '.';
            #else
            cv::String const WinName = "w";
            cv::namedWindow(WinName);
            cv::imshow(WinName, im);
            cv::waitKey(3000);
            #endif
        DoSomethingWithResults(all);
    }//for each frame

    //this is balls but apparently I cant find a goddam c/c++ function I can just call
    //exec because this process no longer needed
    //todo? get fps from cmdline in this prog, and then pass again here instead of hardcode 6?
    static char buf2[512];
    pmsd[-1]='\0';
    sprintf(buf2, "%s.out.mp4", buf);

    pmsd[-1]='z';
    strcpy(pmsd, "%d.png");
    //ffmpeg -r 6 -i 9z%d.png -c:v libx264 output0.mp4

    //ffmpeg wants all spaces

    execl("/usr/bin/ffmpeg",
        "ffmpeg", "-r", "6", "-i", buf, "-c:v", "libx264", buf2,
        (char*) NULL);

    perror("execl");
    return 1;
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
//This is from
//https://www.learnopencv.com/rotation-matrix-to-euler-angles/
static
EulerAnglesF32 rotationMatrixToEulerAngles(const cv::Matx33d &R)
{
    using cv::Vec3d;
    double sy = sqrt(R(0,0) * R(0,0) +  R(1,0) * R(1,0) );

    bool singular = sy < 1e-6; // If

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
    //return RotationMatrix2Euler(rotation_matrix);//openface version
    return rotationMatrixToEulerAngles(rotation_matrix);//learn opencv version
}
