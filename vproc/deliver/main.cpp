/*
    If its run from php, I geuss it puts it in the dir of the php script... IDK

	This takes a video file name, and outputs a processed one
	in the directory of this executable. The processed video name is returned in the status string.
	The status could be something like this:
        tt hello.mp4
    The first char ([0]) is 't' if a video was successfully made.
    The second char ([1]) is 't' if there was a successful transaction commit to the database.
    Then there is a space, followed by the output video name relative to this exe's working dir.
    (it might be different then the input if it has to be encoded differently)

	The video id must also be supplied and it must be a valid reference
	to something already inserted in the table.
	This will update the row with the metadata, after this process has inserted
	all the other stuff. (though if anything fails, nothing is put anywhere, rollback).

	This can be run two ways:

	1:	Just run once, then process ends.
		The format for that is:
			<exe.out> <video_file> <videoid> [shape_predictor trained file]
		The shape predictor arg is optional, defaults to shape_predictor_68_face_landmarks.dat (working dir)
			$ ./vidproc.out /tmp/testvid.mp4 6
		You can also run this from a php script, see php_exec.php for an example.

	2:	Make this process run indefinately, doing initialization only once. This can an improvement
		if using plain cgi. Once this is running as a daemon, it waits for AF_UNIX datagrams.
		Those packets should be a string comprised of a video file name concatenated with a space, then a video id as text.
		See php_af_unix.php for an example. Before you run that though, you need to run this in daemon mode:
			$ ./vidproc.out
		You can also supply the shape_predictor thing if it is not in the working dir of this exe.
*/


//XXX: fallback to handle webm 1000 fps things opencv video capture will go crazy with
//ffmpeg pipe protocol? or at least original make a bunch of image %d files.
//
//For the person doing the database stuff, I put @DB in source code locations in this file and "db.h"
//to point out things of interest.

/*
('X','V','I','D').
.avi
http://answers.opencv.org/question/66545/problems-with-the-video-writer-in-opencv-300/
*/

#include "../library/pch.h"
#include "db.h"

#define STRINGIFY(a) #a
#define STR_UNWRAPPED(a) STRINGIFY(a)

#define ABSOL_STR STR_UNWRAPPED(ABSOL)

#ifndef TRAINED_FILE
    #define TRAINED_FILE STR_UNWRAPPED(ABSOL)
#endif


//good compromise of speed and accuracy for
//webcam style videos, with a single large face
typedef DLibFaceDetectorPyDown<4> DLibFaceDetector;

typedef dlib::point lmCoord;//landmark coordinate pair

//The format returned is [Tx, Ty, Tz, Eul_x, Eul_y, Eul_z]
struct PoseEuler
{
    //cv::Vec3d trans; //And I just discard this.
    EulerAnglesF32 e;//yaw pitch and roll
};

static
PoseEuler getPoseAndDraw(cv::Mat& im, const PairInt16* marks);

static
//void drawDelaunay68(cv::Mat& im, const lmCoord* marks, int, int);
void drawDelaunay68(cv::Mat& im, const PairInt16* lndmks, cv::Rect const rect);

template<class T> T Max(T a, T b) { return b>a ? b : a; }
template<class T> T Min(T a, T b) { return b<a ? b : a; }

static
cv::Rect dlibRectangleToOpenCV(const dlib::rectangle& r)
{
    return cv::Rect(cv::Point2i(r.left(), r.top()), cv::Point2i(r.right() + 1, r.bottom() + 1));
}

/*
    These get-Right/Left-Roi functions cut a rectangle using the 68 points
    gotten from dlib already. The max width and height are taken and expanded a little
    to be conservative (My assumption is its worse to be too small, instead of too big).

    68 point locations reference:
    https://stackoverflow.com/questions/42539240/how-to-get-the-length-of-eyes-and-mouth-using-dlib/44483983#44483983
*/

//left as in looking at the points picture
static
cv::Rect getLeftRoi(const VideoMetadata& dims, const PairInt16* a)
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
cv::Rect getRightRoi(const VideoMetadata& dims, const PairInt16* a)
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

cv::Rect getCrop(dlib::rectangle dr, const VideoMetadata& dims)
{
    cv::Rect r;
    int const wdex= dr.width()/8u;
    int const hgtex= dr.height()/8u;

    r.x = Max(0, int(dr.left())-wdex);
    r.y = Max(0, int(dr.top())-hgtex);

    int const rgt = Min(dims.width-1, int(dr.right())+wdex);//use im dims instead
    int const bot = Min(dims.height-1, int(dr.bottom())+hgtex);

    r.width = rgt - r.x + 1;
    r.height = bot - r.y + 1;
    return r;
}

extern//hume.cpp
cv::Point detectPupilHume(cv::Mat& im, cv::Rect eye);

#define TEST(...)

static
int processVideo(const char* vfilename,
                 std::string& outsref,
                 unsigned videoid,
                 DLibFaceDetector& faceRectFinder,
                 const DLibLandMarkDetector& markDetector,
                 DataBase& db)
{
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
    cv::String voutname = (slash==nullptr ? vfilename : slash+1);

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
        //so if it ends in .webm, will now end in .webm.avi. perhaps change,
        //but can be good that .webm.avi conveys information was sent back in diff fmt
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
    db.begin_transaction_ok();

    cv::Mat im;
    cv::Mat cropmat;

    const cv::Rect croprec_whole(0, 0, vdata.width, vdata.height);

    cv::Rect croprec_A = croprec_whole;
    cv::Rect croprec_B = croprec_whole;

    dlib::cv_image< dlib::bgr_pixel > cimg;
    dlib::rectangle drect_rel2whole;
    dlib::rectangle drect_rel2crop;
    std::vector<dlib::rectangle> faces;
    dlib::full_object_detection detRes;


    FrameResults all={};

    printf("opencv info{fps=%d, w=%d, h=%d}\n", vdata.fps, vdata.width, vdata.height);
    //face detection is the slowest step by far,
    //one of a few optimizations is to not do it every frame
    const int framesPerFaceDetection = vdata.fps/15u + 1u;
    int faceSkips = 0;
    TEST(unsigned facetime=0;)
    fputs("entering read/process loop\n", stderr);
    unsigned long const start_ms = get_millisecs_stamp();
    char status[4] = {'f','f',' ',0};//[0] is video is good, [1] is db transaction commited succesfully


    for (int i=0; i < final_nframes; ++i)
    {
        all.frameno = i;//@DB: zero-based
        all.marks68[0] = {-1, -1};
        all.left_pupil = {-1,-1};
        all.right_pupil = {-1,-1};

        if (!vcap.read(im))
        {
            //if this happens (unlikely if VideoCap opened successfully),
            //smush the video
            //@DB this is where final frames in output may not match input
            fprintf(stderr, "failed to extract frame %d\n", i);
            --final_nframes;
            --i;//then gets inc'd back to same in continue. reuse
            croprec_B = croprec_A = croprec_whole;
            continue;
        }

        if (--faceSkips < 0)//get a new rect...
        {
            TEST(unsigned const t = get_millisecs_stamp();)

            cropmat = im(croprec_B);
            cimg = cropmat;

            faces = faceRectFinder.findFaceRects(cimg);//either whole image or cropmat

            TEST(facetime += get_millisecs_stamp() - t;)

            if (faces.empty())
            {
                fprintf(stderr, "failed to get face ROI for frame %u\n", i);
                //faceSkips = 0;
                croprec_B = croprec_whole;
                goto Lwrite;
            }
            else
            {
                faceSkips = framesPerFaceDetection;

                drect_rel2crop = faces[0];
                drect_rel2whole = dlib::translate_rect(drect_rel2crop, dlib::point(croprec_B.x, croprec_B.y));//NOT SAME!
                //I wrote in a notebook to work this out, but hard to put in ASCII
                croprec_A = croprec_B;
                croprec_B = getCrop(drect_rel2whole, vdata);
            }
        }
        else
        {
            //use previous relative detected rect to previous honing rect
        }

        detRes = markDetector.detectMarks(cimg, drect_rel2crop);//definately pass same rect got from same cimg

        if (detRes.num_parts() != 68)
        {
            fprintf(stderr, "failed to detect 68 marks for frame %u\n", i);
        }
        else
        {
            const lmCoord *const marks = &detRes.part(0);//rel 2 crop
            //1: store marks
            int max_x=vdata.width,
                max_y=vdata.height,
                min_x=max_x,
                min_y=max_y;
            for (int i=0; i!=68; ++i)
            {
                //marks are rel to the cmig passed to sp()
                int const x = marks[i].x() + croprec_A.x;//croprec.x; //+ drect_rel2crop.left();
                int const y = marks[i].y() + croprec_A.y;//croprec.y; //+ drect_rel2crop.top();
                //shape predictor can apparently give a point outside of the picture...
                // ^culprit is face_detector, which can give a rectangle that may not be contained it the original image
                //and you cant insert such a point to ocv's SubDiv2D
                //so figure out what the max size to pass to ocv is
                max_x = Max(max_x, x);
                max_y = Max(max_y, y);

                min_x = Min(min_x, x);
                min_y = Min(min_y, y);

                all.marks68[i] = { (int16_t)x, (int16_t)y };
            }

            auto leyeRect = getLeftRoi(vdata, all.marks68);//THEN THIS IS WRONG
            cv::Point leftEyeCoord = detectPupilHume(im, leyeRect);

            auto reyeRect = getRightRoi(vdata, all.marks68);//THEN THIS IS WRONG
            cv::Point rightEyeCoord = detectPupilHume(im, reyeRect);

            drawDelaunay68(im, all.marks68, {min_x, min_y, max_x-min_x+1, max_y-min_y+1});//do after pupil detection
            //AND HERE
            cv::rectangle(im, dlibRectangleToOpenCV(drect_rel2whole), CV_RGB(255, 255, 0), 2);//thickness
            cv::rectangle(im, croprec_A, CV_RGB(255, 69, 0), 2);
            all.rotation = getPoseAndDraw(im, all.marks68).e;//2: store Euler angles. discard translation...
            ///@TODO: above needs PAir16 param change
            auto const valid=[&vdata](cv::Point p){
                return p.x>=0 && p.x<vdata.width && p.y>=0 && p.x<vdata.height;
            };//end lambda
            //3: store pupils
            //I was drawing them before. If the video is good quality,
            //the persons face is close to the camera and not moving much then it looks good.
            //If not, its too imprecise and is detracting.
            //Regardless, the FrameResults get filled, and that gets sent to where it needs to go.
            if (valid(leftEyeCoord)) {
                //cv::circle(im, leftEyeCoord, 3, CV_RGB(255, 255, 0), CV_FILLED, CV_AA, 0);
                all.left_pupil = {int16_t(leftEyeCoord.x), int16_t(leftEyeCoord.y)};
            }
            //same as above, but for right
            if (valid(rightEyeCoord)) {
                //cv::circle(im, rightEyeCoord, 3, CV_RGB(255, 255, 0), CV_FILLED, CV_AA, 0);
                all.right_pupil = {int16_t(rightEyeCoord.x), int16_t(rightEyeCoord.y)};
            }
        }//end if have 68 points
    Lwrite: //put stuff in db for random access and size consistency,
            //expecting few errors, and successes (common case) will put data for every frame.
        status[0]='t';
        vwriter.write(im);//the return type of this is void
        db.do_something_with_results(all, videoid);//@DB
    }//for each frame

    //if (!vwriter.release()) {}//returns void...
    vwriter.release();

    //@DB: The video metadata can  be sent now
    vdata.nframes = final_nframes;

    db.update_video(vdata, videoid);

    if (db.end_transaction_ok())
        status[1] = 't';

    TEST(printf("face time ms: %u\n", facetime);)

    unsigned long const stop_ms = get_millisecs_stamp();
    printf("Everything except most initialization took about %u ms\n", unsigned(stop_ms-start_ms));

    outsref.clear();
    outsref += (const char *)status;
    outsref += voutname.c_str();
    //printf("%c%c %s", status[0], status[1], voutname.c_str());//put a newline?
    fwrite(outsref.data(), 1, outsref.size(), stdout);
    return 0;
}

static
void fatal(const char *msg)
{
    perror(msg);
    exit(1);
}

#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>

int main(int argc, char **argv)
{
    const char* trained = TRAINED_FILE;
    int videoid = 0;
    bool daemon = false;

    if (argc > 4)
    {
        fputs("Launch once and process video:\n", stderr);
        fprintf(stderr, "\t>> %s <video-file> <video-id> [shape_predictor_absolute]\n", argv[0]);
        fputs("Or to spawn daemon,\n", stderr);
        fprintf(stderr, "\t>> %s [shape_predictor_absolute]\n", argv[0]);
        fputs("\tthen send AF_UNIX DGRAM packets to /tmp/myserver.sock of the form:\n"
              "\t$videofile.' '.$videoid\n", stderr);
        return -1;
    }

    if (argc<=2)
    {
        puts("spawning daemon");
        daemon = true;
        if (argc==2)
            trained = argv[1];
    }
    else
    {
        videoid = atoi(argv[2]);

        if (videoid<1 || videoid>1000)
        {
            fprintf(stderr, "bad video id: %d\n", videoid);
            return -1;
        }

        if (argc==4)
            trained = argv[3];
    }

    DataBase db;
    db.connect_ok();

    DLibFaceDetector faceRectFinder;
    DLibLandMarkDetector markDetector;

    faceRectFinder.init();

    if (markDetector.init(trained)!=0)//I made it print too...
        return -1;

    if (!daemon)
    {
        std::string stdstr;
        return processVideo(argv[1], stdstr, videoid, faceRectFinder, markDetector, db);
    }
    else
    {
        const char SOCKET_FILE[] = "/tmp/myserver.sock";
        int const sock = socket(PF_UNIX, SOCK_DGRAM, 0);
        if (sock < 0)
            fatal("socket()");

        {
            int const optval = 1;
            if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (const char *)&optval , sizeof(int))!=0)
                perror("setsocketopt() failed to set reuse addr");
        }

        sockaddr_un addr_un;
        addr_un.sun_family = AF_UNIX;
        strcpy(addr_un.sun_path, SOCKET_FILE);

        unlink(SOCKET_FILE);
        if (bind(sock, (const struct sockaddr *) &addr_un, sizeof(addr_un)) < 0)
            fatal("bind()");

        enum{BUF_CAP=512};
        char buf[BUF_CAP];

        struct sockaddr_un client_address;
        socklen_t saddrlen = sizeof(struct sockaddr_un);
        std::string sendpkt;
        for (;;)
        {
        	puts("waiting for packet...");
            //sub 1 bucap
            int const ngot = recvfrom(sock, buf, BUF_CAP-1, 0, (struct sockaddr *) &client_address, &saddrlen);
            if (ngot<0){
                perror("recvfrom"); continue; }
            if (ngot==BUF_CAP){
                fputs("packet too long\n", stderr); continue; }

            buf[ngot] = '\0';

            char videofile[256];
            int videoid;
            if (sscanf(buf, "%s %d", (char* )videofile, &videoid)!=2) {
                fprintf(stderr, "packet in bad format: %s\n", buf); continue;  }

            processVideo(videofile, sendpkt, videoid, faceRectFinder, markDetector, db);

            if (sendto(sock, sendpkt.data(), sendpkt.size(), 0, (struct sockaddr *) &client_address, saddrlen) != 2)
                perror("sendto()");
        }
    }

    return 0;
}

//"/tmp/myserver.sock"

/*
    Somehow, shape_predictor can give you a point outside of the rectangle?
    ^ so can face_predictor
    And subdiv will throw an error on insert
*/
void drawDelaunay68(cv::Mat& im, const PairInt16* lndmks, cv::Rect const rect)
//void drawDelaunay68(cv::Mat& im, const lmCoord* lndmks, int max_x, int max_y)
{
    //cv::Size const dims = im.size();
    //cv::Rect const rect = {0,0,dims.width,dims.height};
    //cv::Rect const rect = {0,0,max_x+1,max_y+1};
	cv::Subdiv2D subdiv(rect);//either rect, or points out of range?
	//std::cout << "rect: "<< rect <<'\n';
	for (unsigned i=0; i!=68; ++i)
	{
		PairInt16 const dpt = lndmks[i];
		//std::cout<<"point["<<i<<"] = " << dpt << '\n';
		subdiv.insert(
            {
                float(dpt.x()),
                float(dpt.y())
            });
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
		PairInt16 const dpt = lndmks[i];//TODO, change radius based on dims
		cv::circle(im, cv::Point(dpt.x(), dpt.y()), 2, cv::Scalar(0, 0, 255), CV_FILLED, CV_AA, 0);
	}
}

static inline
cv::Point2d lmc2cv2d(const lmCoord v)
{
    return cv::Point2d(v.x(), v.y());
}

static
EulerAnglesF32 AxisAngle2Euler(const cv::Vec3d& axis_angle);

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

extern
PoseEuler getPoseAndDraw(cv::Mat& im, const PairInt16* marks)
{
    using namespace cv;

    auto const small_to_cvPoint2d = [](PairInt16 a){
        return cv::Point2d(a.x(), a.y());
    };

    const std::array<Point2d, 6> image_points =
    {{
        small_to_cvPoint2d(marks[30]),//Nose tip
        small_to_cvPoint2d(marks[ 8]),//Bottom of Chin
        small_to_cvPoint2d(marks[36]),//Left eye left corner
        small_to_cvPoint2d(marks[45]),//Right eye right corner
        small_to_cvPoint2d(marks[48]),//Right mouth corner
        small_to_cvPoint2d(marks[54])
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

    return PoseEuler{/*translation_vector, */AxisAngle2Euler(rotation_vector)};
}
