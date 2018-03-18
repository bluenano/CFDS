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

static void doSomethingWith68Points(const lmCoord* arr);//no length passed, implied 68


#if 1
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
static void drawDelaunay68(cv::Mat& im, const dlFaceMarkResults& parts);

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

        #define ERROR(M) do{puts(M); return -1;}while(0)
        if (im.empty())
            ERROR("error loading image");

        //convert to dlib fmt. this isnt supposed to allocate or copy
        dlib::cv_image< dlib::bgr_pixel > const cimg(im);
        //more than one face can be detected, so can loop through
        std::vector<dlib::rectangle> const faces = faceRectFinder.findFaceRects(cimg);
        if (faces.empty())
            ERROR("no face detected");
        dlFaceMarkResults const marks = markDetector.detectMarks(cimg, faces[0]);
        if (marks.num_parts()!=68)
            ERROR("num_parts not 68");

        drawDelaunay68(im, marks);

        pmsd[-1] = 'z';//temp overwrite '.' so make new img
        cv::imwrite(buf, im);
        pmsd[-1] = '.';
        #undef ERROR
    }

    return 0;
}

//following is a test to see if the right points are being detected
//I tested it on rob.png from his delauny sample, worked good

//location of all 68 points are known relative to each other, can draw directly
void drawDelaunay68(cv::Mat& im, const dlFaceMarkResults& parts)
{
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

    for (unsigned i=0; i!=68; ++i)
    {
        dlib::point dpt = parts.part(i);

        cv::circle(im, cv::Point(dpt.x(), dpt.y()), 4, cv::Scalar(0, 0, 255), CV_FILLED, CV_AA, 0);
    }
}
#endif

/*
from 3:
Each still image begins with the video ID number followed by a period, a frame number, a period, and the extension png.
For example, still images from a video with video ID 100 saved in the PNG file format would be named as:
100.1.png
100.2.png
100.3.png
*/




