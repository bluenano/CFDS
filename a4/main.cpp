#include "../library/pch.h"

//Should we agree on a place to stuff this?
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

How to handle errors? What if 68 points not found?
--------- Right now I'll just write out the results from the previous frame,
--------- and the "initialization-vector" will be 68 0 0's...
If am already writing points as readable text, guess Ill write -1 -1 for that whole frame.

Also, I think its a good idea to signal how many frames were processed,
right now I do this by process return value. Since the convention is only 0 is success,
nframes - processed is returned. This is meaningless for the optional nframes cmd arg.

Not all usage errors are checked for, as it is assumed process starter will
launch with correct arguments.

This may not be the best way to do things, but I tried to follow the assignment specs,
and serves as reference material for trying something else.

Also, I can get delauny triangles working too, see second half of file.
*/
typedef dlib::point lmCoord;//lanmark coord

#if 0
static void doSomethingWith68Points(const lmCoord* arr);//no length passed, implied 68

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
    char buf[BufCap];
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

    unsigned const iend = nframes + 1u;
    for (unsigned i=1u; i!=iend; ++i)//note 1-based
    {
        sprintf(pmsd, "%u.png", i);//quick and dirty
        cv::Mat im = cv::imread(buf);//this return style means it allocates every time...
        if (im.empty())
            return iend - i;

        //convert to dlib fmt. this isnt supposed to allocate or copy
        dlib::cv_image< dlib::bgr_pixel > const cimg(im);
        //more than one face can be detected, so can loop through
        std::vector<dlib::rectangle> const faces = faceRectFinder.findFaceRects(cimg);
        if (!faces.empty())
        {
            dlFaceMarkResults const marks = markDetector.detectMarks(cimg, faces[0]);
            if (marks.num_parts()!=68)
                goto Lerror;
            doSomethingWith68Points(&marks.part(0));//"hack" to get at std::vector.data()
        }
        else
        {
        Lerror:
            puts("-1 -1");
        }
    }

    return 0;
}
//for the assignment, should this be some kind of DB thing?
//pretty sure he said flat file is OK and preferred.
//I'd still rather do all the processing in one shot instead
//of many intermediate i/o steps
static void doSomethingWith68Points(const lmCoord* arr)//no length passed, implied 68
{
    unsigned i=0;
    do{
        dlib::point const v = arr[i];
        printf("%u %u\n", (unsigned)v.x(), (unsigned)v.y());
    }while(++i!=68);
}

/*
    This program is like the above,
    but instead of writing out the point locations
    it makes a new image for each, where the first '.'
    (not the .png) is replaced with 'z'. A delauny triangulation
    is drawn on the new image files.

    TODO: in final, make radius of circles dependent on image dimensions.
*/

#else
#define ERROR(M) do{puts(M); return -1;}while(0)
static void drawDelaunay68(cv::Mat& im, const lmCoord* marks);

// The format returned is [Tx, Ty, Tz, Eul_x, Eul_y, Eul_z]
struct PoseEuler
{
    cv::Vec3d trans, rot_eul;
};
static PoseEuler getPoseAndDraw(cv::Mat& im, const lmCoord* marks);

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
    char buf[BufCap];
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

    for (unsigned i=1u; i<=nframes; ++i)//note 1-based
    {
        sprintf(pmsd, "%u.png", i);//quick and dirty
        cv::Mat im = cv::imread(buf);//this return style means it allocates every time...


        if (im.empty())
            ERROR("error loading image");

        //convert to dlib fmt. this isnt supposed to allocate or copy
        dlib::cv_image< dlib::bgr_pixel > const cimg(im);
        //more than one face can be detected, so can loop through
        std::vector<dlib::rectangle> const faces = faceRectFinder.findFaceRects(cimg);
        if (faces.empty())
            ERROR("no face detected");
        dlFaceMarkResults const detRes = markDetector.detectMarks(cimg, faces[0]);
        if (detRes.num_parts()!=68)
            ERROR("num_parts not 68");

        const lmCoord *const marks = &detRes.part(0);

        drawDelaunay68(im, marks);
        //The return value is not in the format of 3 euler angles
        //Is drawing the projection enough?
        //If we need the points somewhere
        (void)getPoseAndDraw(im, marks);
        /*This throws, cant specify writer?
        PoseEuler const v = getPoseAndDraw(im, marks);

        //CHECK IF IMAGE NOT TOO SMALL
        char buf[512];
        sprintf(buf, "Tx=%3.3f, Ty=%3.3f, Tz=%3.3f\nyaw=%3.3f, pitch=%3.3f, roll=%3.3f",
                v.trans[0], v.trans[1], v.trans[2], v.rot_eul[0], v.rot_eul[1], v.rot_eul[2]);

        const int fontFace = cv::FONT_HERSHEY_SCRIPT_SIMPLEX;
        const double fontScale = 2;
        const int thickness = 3;
        const cv::Point textOrg(4, 4);
        cv::putText(im, buf, textOrg, fontFace, fontScale, cv::Scalar::all(255), thickness, 8);
        */

        pmsd[-1] = 'z';//temp overwrite '.' so make new img
        cv::imwrite(buf, im);
        pmsd[-1] = '.';
    }

    return 0;
}

//following is a test to see if the right points are being detected
//I tested it on rob.png from his delauny sample, worked good

//location of all 68 points are known relative to each other, can draw directly
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

cv::Vec3d AxisAngle2Euler(const cv::Vec3d& axis_angle);

static PoseEuler getPoseAndDraw(cv::Mat& im, const lmCoord* marks)
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
        circle(im, point, 4, cv::Scalar(0, 255, 0), CV_FILLED, CV_AA, 0);

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

    cv::line(im, image_points[0], nose_end_point2D[0], cv::Scalar(0,255,255), 2);

    return PoseEuler{translation_vector, AxisAngle2Euler(rotation_vector)};
}
#endif


#if 1
//The following two functions are pasted from OpenFace

// Using the XYZ convention R = Rx * Ry * Rz, left-handed positive sign
cv::Vec3d RotationMatrix2Euler(const cv::Matx33d& rotation_matrix)
{
    double q0 = sqrt(1 + rotation_matrix(0, 0) + rotation_matrix(1, 1) + rotation_matrix(2, 2)) / 2.0;
    double q1 = (rotation_matrix(2, 1) - rotation_matrix(1, 2)) / (4.0*q0);
    double q2 = (rotation_matrix(0, 2) - rotation_matrix(2, 0)) / (4.0*q0);
    double q3 = (rotation_matrix(1, 0) - rotation_matrix(0, 1)) / (4.0*q0);

    //double t1 = 2.0 * (q0*q2 + q1*q3);//unused variable?

    double yaw = asin(2.0 * (q0*q2 + q1*q3));
    double pitch = atan2(2.0 * (q0*q1 - q2*q3), q0*q0 - q1*q1 - q2*q2 + q3*q3);
    double roll = atan2(2.0 * (q0*q3 - q1*q2), q0*q0 + q1*q1 - q2*q2 - q3*q3);

    return cv::Vec3d(pitch, yaw, roll);
}

//This is from
//https://www.learnopencv.com/rotation-matrix-to-euler-angles/
cv::Vec3d rotationMatrixToEulerAngles(const cv::Matx33d &R)
{
    using cv::Vec3d;
    double sy = sqrt(R(0,0) * R(0,0) +  R(1,0) * R(1,0) );

    bool singular = sy < 1e-6; // If

    if (!singular)
        return Vec3d(atan2(R(2,1) , R(2,2)),
                     atan2(-R(2,0), sy),
                     atan2(R(1,0), R(0,0)));
    else
        return Vec3d(atan2(-R(1,2), R(1,1)),
                          atan2(-R(2,0), sy),
                          0.0);
}

cv::Vec3d AxisAngle2Euler(const cv::Vec3d& axis_angle)
{
    cv::Matx33d rotation_matrix;
    cv::Rodrigues(axis_angle, rotation_matrix);
    //return RotationMatrix2Euler(rotation_matrix);//openface version
    return rotationMatrixToEulerAngles(rotation_matrix);//learn opencv version
}

#endif

#if 0
    cv::solvePnP(landmarks_3D, landmarks_2D, camera_matrix, cv::Mat(), vec_rot, vec_trans, true);

    cv::Vec3d euler = Utilities::AxisAngle2Euler(vec_rot);

    return cv::Vec6d(vec_trans[0], vec_trans[1], vec_trans[2], euler[0], euler[1], euler[2]);

#endif // 0

/*
from 3:
Each still image begins with the video ID number followed by a period, a frame number, a period, and the extension png.
For example, still images from a video with video ID 100 saved in the PNG file format would be named as:
100.1.png
100.2.png
100.3.png

OpenCV Error: Unspecified error (could not find a writer for the specified extension) in imwrite_, file /home/jw/opencv/opencv-3.3.0/modules/imgcodecs/src/loadsave.cpp, line 604
terminate called after throwing an instance of 'cv::Exception'
  what():  /home/jw/opencv/opencv-3.3.0/modules/imgcodecs/src/loadsave.cpp:604: error: (-2) could not find a writer for the specified extension in function imwrite_

*/



