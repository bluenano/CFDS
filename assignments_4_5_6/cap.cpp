#ifndef CAP

/*
    Tested on
    opencv 3.3 built with ffmpeg/libav
    linux mint 18.3

    Seems to work with .mp4 and .webm
*/

/*
CV_CAP_PROP_FRAME_WIDTH Width of the frames in the video stream.
CV_CAP_PROP_FRAME_HEIGHT Height of the frames in the video stream.
CV_CAP_PROP_FPS Frame rate.
*/

#include "../library/pch.h"

int main(int argc, char** argv)
{
    using namespace cv;

    if (argc!=2) {fputs("no video filename\n", stderr); return -1; }

    VideoCapture cap(argv[1]); // open the video file
    if(!cap.isOpened())  // check if we succeeded
        {fprintf(stderr, "could not open video file: [%s]\n", argv[1]); return -1; }


    cv::String const WinName = "w";
    cv::namedWindow(WinName);

    int const nframes = cap.get(CV_CAP_PROP_FRAME_COUNT);
    int const fps = cap.get(CV_CAP_PROP_FPS);
    int const width = cap.get(CV_CAP_PROP_FRAME_WIDTH);
    int const height = cap.get(CV_CAP_PROP_FRAME_HEIGHT);

    cv::Mat im;
    printf("nframes: %d\n"
           "fps: %d\n"
           "width: %d\n"
           "height: %d\n", nframes, fps, width, height);

    for(int i = 0; i < nframes; i++)
    {
        if (cap.read(im))
        {
            cv::Size dim = im.size();
            if (dim.width != width || dim.height !=height)
                fputs("seems to be an inconsistency\n", stderr);

            cvtColor(im, im, CV_BGR2GRAY);
            cv::imshow(WinName, im);
            cv::waitKey(3000);
        }
        else
            fprintf(stderr, "failed to read frame %d\n", i);
    }

    return 0;
}
#endif // CAP
