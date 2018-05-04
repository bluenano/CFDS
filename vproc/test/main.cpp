

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
    Circumvent slow dlib init:
    Shared lib?
    Daemon process (so initialization done once per system start),
    and communicate via ipc, like AF_UNIX udp example?

    Possible optimizations:

    The bounding face ROI detection is actually much slower than the subsequent
    68 point detection. If the fps is >= 24 or so, could use the same dimensions
    from the previous detection every other time, skipping the call.

    Also, downscale image and detect rect on that, then scale rect.

    Two threads: A, B.
    r = VideCapture.extract
    f = detect face rect (reportedly slower than other operations)
    p = other processing (68 landmark detection, drawing)
    w = VideoWriter.append
    ~ = idle

    A: rfffppw...rfffppw~
    B: ~rfffppw...rfffppw

    But can also apply every other frame rect to this threading strategy?
    ^yes, double up r, but tradeoff
    rrfffppwfffppw...
    ~~rrfffppwfffppw
*/
#include "../library/pch.h"

//#include "db.h"

#ifndef TFILE
    #define TFILE "./shape_predictor_68_face_landmarks.dat"
#endif

#ifndef _POSIX_C_SOURCE
    #define _POSIX_C_SOURCE 200809L
#elif (_POSIX_C_SOURCE) < 199309L
    #error compile with -D_POSIX_C_SOURCE=200809L
#endif

inline
unsigned long get_millisecs_stamp(void)
{
    struct timespec spec;

    clock_gettime(CLOCK_MONOTONIC, &spec);

    return spec.tv_sec*1000u + spec.tv_nsec/(1000u*1000u);//10e6 millisecs per nanosec
}

void millisleep(unsigned long msecs)
{
    struct timespec a[2];//req, rem

    if (msecs >= 1000u)
    {
        a[0].tv_sec = msecs/1000u;
        a[0].tv_nsec = msecs*1000u;//only by one 1000 because secs consumed
    }
    else
    {
        a[0].tv_sec = 0;
        a[0].tv_nsec = msecs*(1000u*1000u);
    }
    #if 0//untested
    unsigned i=0;
    while (nanosleep(&a[i], &a[i^1])!=0 && errno==EINTR)
        i^=1;
    #else
    nanosleep(a+0, a+1);//const request, remaining (due to thread signal/intr) not a concern for this assignment
    #endif
}

#include <stdint.h>

/*
    These are gathered now also; once at the start of the process.

    nframes in th output video is not final until all computation is done.
*/
struct VideoMetadata
{
    int32_t nframes;
    int32_t fps;
    int32_t width;
    int32_t height;
};

static
cv::Rect dlibRectangleToOpenCV(const dlib::rectangle& r)
{
    return cv::Rect(cv::Point2i(r.left(), r.top()), cv::Point2i(r.right() + 1, r.bottom() + 1));
}

typedef dlib::point lmCoord;//landmark coordinate pair

static
void drawDelaunay68(cv::Mat& im, const lmCoord* marks);


//regular 6_5 is like 2.5 times slower than 2_1,
//but not as accurate, compromise with 4_3
//also: honing-in idea may help with this also
typedef DLibFaceDetectorPyDown<4> FaceFinder;

//CV_CAP_PROP_POS_FRAMES 0-based index of the frame to be decoded/captured next.
static inline//face rect finder not const
long int meat(const char * vfilename, FaceFinder& faceRectFinder, const DLibLandMarkDetector& markDetector)
{
    unsigned long faceTime = 0u;

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
    fprintf(stderr, "nframes according to opencv: %u\n", final_nframes);

    enum{MAX_FPS=30};

    if (vdata.fps > MAX_FPS)//this VideoCapture...
    {
        puts("reducing fps");
        if (!vcap.set(CV_CAP_PROP_FPS, MAX_FPS) || int(vdata.fps = vcap.get(CV_CAP_PROP_FPS)) != MAX_FPS )
        {
            fprintf(stderr, "failed to set fps to 30 from vdata.fps: %d\n", vdata.fps);
            return -1;
        }
    }

    //fprintf(stderr, "%lf\n", vcap.get(CV_CAP_PROP_FOURCC));

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

    fputs("entering read/process loop\n", stderr);

    cv::Mat im;
    dlib::rectangle faceroi;
    bool useprev = false;

    int opencvfails = 0;
    int i = 0;
    while (i < final_nframes && opencvfails < 50)
    {
        if (!vcap.read(im))
        {
            --final_nframes;
            ++opencvfails;
            millisleep(2);
            continue;
        }

        ++i;

        //convert to dlib fmt. this isnt supposed to allocate or copy
        dlib::cv_image< dlib::bgr_pixel > const cimg(im);
        dlib::full_object_detection detRes;
        //dlib::cv_image<unsigned char> const cimg(im);
        //more than one face can be detected, so can loop through

        if (!useprev)//need grab on i=0
        {
            unsigned long const tstart = get_millisecs_stamp();
            std::vector<dlib::rectangle> const faces = faceRectFinder.findFaceRects(cimg);
            faceTime += get_millisecs_stamp() - tstart;

            if (faces.empty())
            {
                fprintf(stderr, "failed to get face ROI for frame %d\n", i-1);
                goto L_write;
            }

            faceroi = faces[0];
            useprev = true;
        }
        else
            useprev = false;

        detRes = markDetector.detectMarks(cimg, faceroi);

        if (detRes.num_parts() != 68)
        {
            fprintf(stderr, "failed to detect 68 marks for frame %u\n", i);
        }
        else
        {
            const lmCoord *const marks = &detRes.part(0);

            drawDelaunay68(im, marks);
            cv::rectangle(im, dlibRectangleToOpenCV(faceroi), CV_RGB(255, 255, 0), 2);//thickness
        }//end if have 68 points
    L_write:
        vwriter.write(im);
    }//for each frame

    vwriter.release();
    if (opencvfails)
        fprintf(stderr,"VideoCapture failed to read %u frames.\n", opencvfails);
    return faceTime;
}

int main(int argc, char **argv)
{
    const char *shapeloc = TFILE;

    if (argc==2)
        shapeloc = argv[1];

    FaceFinder faceRectFinder;
    DLibLandMarkDetector markDetector;

    if (markDetector.init(shapeloc)!=0)//I made it print too...
        return -1;

    char buf[128];
    while (fputs("file: ", stdout), scanf("%124s", (char *)buf)==1)
    {
        unsigned long alltime = get_millisecs_stamp();
        unsigned long const facetime = meat(buf, faceRectFinder, markDetector);
        alltime = get_millisecs_stamp() - alltime;

        printf("face_time: %lu, total: %lu\n", facetime, alltime);
    }

    return 0;
}
//location of all 68 points are known relative to each other, could draw directly
//EDIT: NEED TO FIX THIS LIKE IN MAIN DELIVER,
//will fail on some videos where face detector gives
//a rectangle that goes outside of the original image
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

    /*
    if (argc!=2)
    {
        fprintf(stderr, "usage: %s <video-file>\n", argv[0]);
        return -1;
    }
    */

/* 6 to 5
file: /home/jw/Data/7.mp4
nframes according to opencv: 40
OpenCV: FFMPEG: tag 0x31637661/'avc1' is not supported with codec id 28 and format 'mp4 / MP4 (MPEG-4 Part 14)'
OpenCV: FFMPEG: fallback to use tag 0x00000021/'!???'
out_7.mp4
entering read/process loop
face_time: 867, total: 1502
file: /home/jw/Data/7.mp4
nframes according to opencv: 40
OpenCV: FFMPEG: tag 0x31637661/'avc1' is not supported with codec id 28 and format 'mp4 / MP4 (MPEG-4 Part 14)'
OpenCV: FFMPEG: fallback to use tag 0x00000021/'!???'
out_7.mp4
entering read/process loop
face_time: 865, total: 1477
file: /home/jw/Data/7.mp4
nframes according to opencv: 40
OpenCV: FFMPEG: tag 0x31637661/'avc1' is not supported with codec id 28 and format 'mp4 / MP4 (MPEG-4 Part 14)'
OpenCV: FFMPEG: fallback to use tag 0x00000021/'!???'
out_7.mp4
entering read/process loop
face_time: 863, total: 1465
file:
*/

/* 4 to 3:
file: /home/jw/Data/7.mp4
nframes according to opencv: 40
OpenCV: FFMPEG: tag 0x31637661/'avc1' is not supported with codec id 28 and format 'mp4 / MP4 (MPEG-4 Part 14)'
OpenCV: FFMPEG: fallback to use tag 0x00000021/'!???'
out_7.mp4
entering read/process loop
face_time: 590, total: 1266
file: /home/jw/Data/7.mp4
nframes according to opencv: 40
OpenCV: FFMPEG: tag 0x31637661/'avc1' is not supported with codec id 28 and format 'mp4 / MP4 (MPEG-4 Part 14)'
OpenCV: FFMPEG: fallback to use tag 0x00000021/'!???'
out_7.mp4
entering read/process loop
face_time: 591, total: 1216
file: /home/jw/Data/7.mp4
nframes according to opencv: 40
OpenCV: FFMPEG: tag 0x31637661/'avc1' is not supported with codec id 28 and format 'mp4 / MP4 (MPEG-4 Part 14)'
OpenCV: FFMPEG: fallback to use tag 0x00000021/'!???'
out_7.mp4
entering read/process loop
face_time: 599, total: 1196
file:
*/

/* 2 to 1:
file: /home/jw/Data/7.mp4
nframes according to opencv: 40
OpenCV: FFMPEG: tag 0x31637661/'avc1' is not supported with codec id 28 and format 'mp4 / MP4 (MPEG-4 Part 14)'
OpenCV: FFMPEG: fallback to use tag 0x00000021/'!???'
out_7.mp4
entering read/process loop
face_time: 316, total: 946
file: /home/jw/Data/7.mp4
nframes according to opencv: 40
OpenCV: FFMPEG: tag 0x31637661/'avc1' is not supported with codec id 28 and format 'mp4 / MP4 (MPEG-4 Part 14)'
OpenCV: FFMPEG: fallback to use tag 0x00000021/'!???'
out_7.mp4
entering read/process loop
face_time: 303, total: 924
file: /home/jw/Data/7.mp4
nframes according to opencv: 40
OpenCV: FFMPEG: tag 0x31637661/'avc1' is not supported with codec id 28 and format 'mp4 / MP4 (MPEG-4 Part 14)'
OpenCV: FFMPEG: fallback to use tag 0x00000021/'!???'
out_7.mp4
entering read/process loop
face_time: 311, total: 930
file:
*/
