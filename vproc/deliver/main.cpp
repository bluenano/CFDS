/*
    @sean: here is sample output and commands.
    ran this manually, but tested calling from php too.
    also, see my vproc/deldata.php, which has a function that can be called
    to delete the corresponding file.

jw@jw-laptop ~/CS160Project/bin $ ./vidproc.out ../videos/testvid.mp4 888
OpenCV: FFMPEG: tag 0x31637661/'avc1' is not supported with codec id 28 and format 'mp4 / MP4 (MPEG-4 Part 14)'
OpenCV: FFMPEG: fallback to use tag 0x00000021/'!???'
processing file:
../videos/testvid.mp4
with video id: 888
Opening "wb":
  [/home/jw/Sites/cs160/vdata/vdata_888.dat]
opencv info{fps=17, w=640, h=480}
Entering read/process/write loop...
file filled and closed successfully
face time ms: 130
Everything except most initialization took about 804 ms
tt testvid.mp4jw@jw-laptop ~/CS160Project/bin $ #look at video
jw@jw-laptop ~/CS160Project/bin $ cd ../vproc/fs
jw@jw-laptop ~/CS160Project/vproc/fs $ ./fs.out #NO ARGS... i may change this
enter a file to inspect: /home/jw/Sites/cs160/vdata/vdata_888.dat
looking for file:
  [/home/jw/Sites/cs160/vdata/vdata_888.dat]
nframes: 40,
fps: 17,
w: 640,
h: 480
enter a valid 0-indexed frame to inspect, i for next sequential, or q to break
>> i
364,213 365,236 371,259 377,282 385,307 398,329 417,347 439,362
465,365 491,361 515,346 534,328 549,305 557,280 561,255 564,228
563,203 372,192 381,177 398,173 417,174 434,180 464,175 484,166
506,162 528,166 544,180 450,198 451,220 451,242 451,264 436,273
444,277 454,280 465,276 474,271 390,208 400,202 413,201 426,206
414,210 401,212 482,203 495,195 509,194 521,199 510,204 496,204
422,305 433,299 446,295 457,298 468,294 483,297 497,301 484,314
470,319 458,321 446,320 434,315 426,305 446,304 457,306 469,303
491,302 469,304 458,307 446,305
   ... are the (x,y) 68 landmark coords.
left:  {408, 203}
right: { -1,  -1}
y,p,r: {-3.058731, 0.433284, -0.073931}
>> q
enter a file to inspect: ^Z
[1]+  Stopped                 ./fs.out
jw@jw-laptop ~/CS160Project/vproc/fs $

*/

/*
    For @Bruce: I commented parts of interest with @Requirement
    that you can search for if that helps.

    Type make in this directory to build,
    after installing necessary components.
    See the README for how to get those: ffmpeg, opencv, and dlib.

	This exe takes a video file name, and outputs a processed one
	in the directory of this executable, or where it may be called from php.
	The processed video name is returned in the status string.
	The status could be something like this:
        tt hello.mp4
    The first char ([0]) is 't' if a video was successfully made.
    The second char ([1]) is 't' if there was a successful full write of all the video data to the file:
        ~/Sites/cs160/vdata/vdata_123.dat
    (where 123 would be the video id).
    Then there is a space, followed by the output video name relative to this exe's working dir.
    (it might be different then the input if it has to be encoded differently)

	The video id must also be supplied.

	This can be run two ways:

	1:	Just run once, then process ends.
		The format for that is:
			<exe.out> <video_file> <videoid> [shape_predictor trained file]
		The shape predictor arg is optional, and can be supplied for backwards compat, but should build this with -DABSOL
			$ ./vidproc.out /tmp/testvid.mp4 6
		You can also run this from a php script, see php_exec.php for an example.

	2:	Make this process run indefinitely, doing initialization only once. This can an improvement
		if using plain cgi. Once this is running as a daemon, it waits for AF_UNIX datagrams.
		Those packets should be a string comprised of an
		ABSOLUTE video file name concatenated with a space, then a video id as text.
		See php_af_unix.php for an example. Before you run that though, you need to run this in daemon mode:
			$ ./vidproc.out
*/

#include "../library/pch.h"
#include "../fs/fs.h"
#include "util.h"

//#make MFLAGS='-mavx2 -DABSOL="/home/jw/CS160Project/data/shape_predictor_68_face_landmarks.dat"'

#ifndef TRAINED_FILE
    #ifdef ABSOL
        #define TRAINED_FILE STRINGIFY(ABSOL)
    #else
    #warn ABSOL not defined, using cwd
        #define TRAINED_FILE "/home/jw/CS160Project/data/shape_predictor_68_face_landmarks.dat"
    #endif // ABSOL
#endif

//good compromise of speed and accuracy for
//webcam style videos, with a single large face
typedef DLibFaceDetectorPyDown<4> DLibFaceDetector;

//The format returned is [Tx, Ty, Tz, Eul_x, Eul_y, Eul_z]
struct PoseEuler
{
    //cv::Vec3d trans; //And I just discard this.
    EulerAnglesF32 e;//yaw pitch and roll, defined in fs.h
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

//expand previously detected rect returned by face_detector()
//by 1/8 on each side for both dimensions
cv::Rect getCrop(dlib::rectangle dr, const VideoMetadata& dims)
{
    cv::Rect r;
    int const wdex= dr.width()/8u;
    int const hgtex= dr.height()/8u;

    r.x = Max(0, int(dr.left())-wdex);
    r.y = Max(0, int(dr.top())-hgtex);

    int const rgt = Min(dims.width-1, int(dr.right())+wdex);
    int const bot = Min(dims.height-1, int(dr.bottom())+hgtex);

    r.width = rgt - r.x + 1;
    r.height = bot - r.y + 1;
    return r;
}

extern//hume.cpp
cv::Point detectPupilHume(cv::Mat& im, cv::Rect eye);

#define TEST(...) __VA_ARGS__

static
int processVideo(const char* vfilename,
                 std::string& outsref,
                 int videoid,
                 DLibFaceDetector& faceRectFinder,
                 const DLibLandMarkDetector& markDetector)
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
    //fprintf(stderr, "%lf\n", vcap.get(CV_CAP_PROP_FOURCC));

    const char * slash = strrchr(vfilename, '/');//reverse
    cv::String voutname = (slash==nullptr ? vfilename : slash+1);

    cv::VideoWriter vwriter(voutname,
                            vcap.get(CV_CAP_PROP_FOURCC),
                            vdata.fps,
                            cv::Size(vdata.width, vdata.height));
    if (!vwriter.isOpened())
    {
        //Trying this is better than nothing.
        fprintf(stderr, "failed create video writer in uploaded format,\nattempting to convert to avi\n");
        voutname += ".avi";
        //Note:
        //So if it ends in .webm, will now end in .webm.avi. perhaps change,
        //but can be good that .webm.avi conveys information was sent back in diff fmt
        //this is way the output video name is in the status string.
        if (!vwriter.open(  voutname,
                            877677894.0,//vcap.get(cv::VideoWriter::fourcc()),
                            vdata.fps,
                            cv::Size(vdata.width, vdata.height)))
        {
            fputs("could not convert video for writer, exiting\n", stderr);
            return -1;
        }
    }

    printf("processing file:\n%s\nwith video id: %d\n", vfilename, videoid);
    VideoDataFiler db;
    //if theres an error, a flag is set, and everything else is a noop
    //either everything goes in, or nothing. a 'rollback' is deleting the file.
    //status is returned in [1], 't' means OK
    db.begin_transaction_ok(videoid);
    db.write_header_ok(vdata);

    cv::Mat im;
    cv::Mat cropmat;

    const cv::Rect croprec_whole(0, 0, vdata.width, vdata.height);

    cv::Rect croprec_A;
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
    fputs("Entering read/process/write loop...\n", stderr);
    unsigned long const start_ms = get_millisecs_stamp();
    char status[4] = {'f','f',' ',0};//[0] is video is good, [1] is file transaction

    int cvfails = 0;

    for (int i=0; i < vdata.nframes; ++i)
    {
        all.marks68[0] = {-1, -1};
        all.left_pupil = {-1,-1};
        all.right_pupil = {-1,-1};

        if (!vcap.read(im))
        {
            fprintf(stderr, "failed to extract frame (i: %d - prev_vcap_fails: %d)\n", i, cvfails++);
            croprec_B = croprec_A = croprec_whole;
            continue;
        }

        if (--faceSkips < 0)//get a new detected rect
        {
            TEST(unsigned const t = get_millisecs_stamp();)

            cropmat = im(croprec_B);
            cimg = cropmat;

            faces = faceRectFinder.findFaceRects(cimg);

            TEST(facetime += get_millisecs_stamp() - t;)

            if (faces.empty())
            {
                fprintf(stderr, "failed to get face ROI for frame i: %d, prev_vcap_fails: %d\n", i, cvfails);
                croprec_B = croprec_whole;                              //don't inc here, not a vcap problem
                //faceSkips = 0;// signed and < compare
                goto Lwrite;
            }
            else
            {
                faceSkips = framesPerFaceDetection;

                drect_rel2crop = faces[0];
                drect_rel2whole = dlib::translate_rect(drect_rel2crop, dlib::point(croprec_B.x, croprec_B.y));

                croprec_A = croprec_B;
                croprec_B = getCrop(drect_rel2whole, vdata);
            }
        }
        else
        {
            //use previous crop and face detection
        }

        detRes = markDetector.detectMarks(cimg, drect_rel2crop);//definitely pass same rect got from same cimg

        if (detRes.num_parts() != 68)
        {
            fprintf(stderr, "failed to detect 68 marks for frame %u\n", i);
        }
        else
        {
            const dlib::point *const marks = &detRes.part(0);//rel 2 crop
            int max_x=vdata.width,
                max_y=vdata.height,
                min_x=max_x,
                min_y=max_y;
            for (int i=0; i!=68; ++i)
            {
                //marks are rel to the cmig passed to sp()
                int const x = marks[i].x() + croprec_A.x;
                int const y = marks[i].y() + croprec_A.y;
                //see notes above drawDelaunay68() for an explanation of why following is needed.
                max_x = Max(max_x, x);
                max_y = Max(max_y, y);

                min_x = Min(min_x, x);
                min_y = Min(min_y, y);
                ///@Requirement: store marks
                all.marks68[i] = { (int16_t)x, (int16_t)y };
            }
            ///@Requirement: get pupils
            auto leyeRect = getLeftRoi(vdata, all.marks68);
            cv::Point leftEyeCoord = detectPupilHume(im, leyeRect);

            auto reyeRect = getRightRoi(vdata, all.marks68);
            cv::Point rightEyeCoord = detectPupilHume(im, reyeRect);
            ///@Requirement: draw Delaunay
            drawDelaunay68(im, all.marks68, {min_x, min_y, max_x-min_x+1, max_y-min_y+1});//do after pupil detection
            //draw both the detected rectangle and the crop, just to show we can code things
            cv::rectangle(im, dlibRectangleToOpenCV(drect_rel2whole), CV_RGB(255, 255, 0), 2);//thickness
            cv::rectangle(im, croprec_A, CV_RGB(255, 69, 0), 2);
            ///@Requirement: get pose
            all.rotation = getPoseAndDraw(im, all.marks68).e;//discard translation

            auto const valid=[&vdata](cv::Point p){
                return p.x>=0 && p.x<vdata.width && p.y>=0 && p.x<vdata.height;
            };
            //now actually store pupils, if not detected, previous {-1,-1} set will remain filled, signaling error
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
    Lwrite: //put stuff in file for random access and size consistency,
            //expecting few errors, and successes (common case) will put data for every frame.
        db.output_frame(all);///@Requirement: store in a file or database
        status[0]='t';//at least one frame made it into the video
        vwriter.write(im);//the return type of this is void
    }//for each frame

    vwriter.release();//returns void

    if (cvfails > 0)
    {
        vdata.nframes -= cvfails;
        db.fixup_header_ok(vdata);
    }

    if (db.end_transaction_ok())
        status[1] = 't';

    TEST(printf("face time ms: %u\n", facetime);)

    unsigned long const stop_ms = get_millisecs_stamp();
    printf("Everything except most initialization took about %u ms\n", unsigned(stop_ms-start_ms));

    outsref.clear();
    outsref += (const char *)status;
    outsref += voutname.c_str();
    //put a newline?
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

    DLibFaceDetector faceRectFinder;
    DLibLandMarkDetector markDetector;

    faceRectFinder.init();

    if (markDetector.init(trained)!=0)//I made it print too...
        return -1;

    if (!daemon)
    {
        std::string stdstr;
        return processVideo(argv[1], stdstr, videoid, faceRectFinder, markDetector);
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

            processVideo(videofile, sendpkt, videoid, faceRectFinder, markDetector);

            if (sendto(sock, sendpkt.data(), sendpkt.size(), 0, (struct sockaddr *) &client_address, saddrlen) != 2)
                perror("sendto()");
        }
    }

    return 0;
}

/*
    Importance of testing for robust software (and also reading api docs carefully):

    dlib's face_detector (aka object_detector, gives face ROI)
    and shape_predictor (gives 68 landmarks)
    can return a rect or points not contained within the original image.
    However, cv::SubDiv2D::insert contract is the point must be contained in the
    rect passed to the ctor or set via another method.

    So, it is incorrect to pass {0, 0 frame width, frame height} (w and h probed at start).
    To SubDiv.
*/
void drawDelaunay68(cv::Mat& im, const PairInt16* lndmks, cv::Rect const rect)
{
	cv::Subdiv2D subdiv(rect);
	for (unsigned i=0; i!=68; ++i)
	{
		PairInt16 const dpt = lndmks[i];
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
		PairInt16 const dpt = lndmks[i];
		cv::circle(im, cv::Point(dpt.x(), dpt.y()), 2, cv::Scalar(0, 0, 255), CV_FILLED, CV_AA, 0);
	}
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

    return rotationMatrixToEulerAngles(rotation_matrix);
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
