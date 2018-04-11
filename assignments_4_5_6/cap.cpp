//this is just an example of what another process(in a diff langauge) could do with the data
//probably better to figure out whatever is needed to do and add it into the video processing cpp src.

//nvm... this is bogus, ndata <=0...

#include "header.h"

#include <stdio.h>
#include <stdlib.h>

int main()
{
    //hard coded...
    FILE* const fpin = popen("./vprocess.out contour.mp4", "r");
    if (!fpin) {perror("popen()"); return -1; }

    VideoMetadata meta={};
    fread(&meta, sizeof(VideoMetadata), 1, fpin);

    if (meta.nframes <= 0)
        {perror("bad nframes"); return -1; }//free fp


    FrameResults *const res = (FrameResults *) malloc(meta.nframes * sizeof(FrameResults));

    fread(res, sizeof(FrameResults), meta.nframes, fpin);

    printf("nframes: %d\n"
           "fps: %d\n"
           "width: %d\n"
           "height: %d\n", meta.nframes, meta.fps, meta.width, meta.height);



    puts("The 68 points for frame[0] are:");
    for (int i=0; i!=68; ++i)
        printf("{%u,%u}", res[0].marks68[i].x(), res[0].marks68[i].y());

    //os does this
    free(res);
    pclose(fpin);
    return 0;
}


#if 0

/*
    Tested on
    opencv 3.3 built with ???
    linux mint 18.3

    Seems to work with .mp4 and .webm<-reading only
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
#endif
