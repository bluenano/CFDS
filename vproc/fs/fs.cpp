/*
    Use the makefile to build. I just learned about $HOME ...
    If no arg, will prompt for file to look at. (why did I do this, instead of cmdline arg?)
    Else, will make file with some data that can be seen at start of main, with videoid = atoi(argv[1]);

    video data files are named like:
        ~/Sites/cs160/vdata/vdata_123.dat
*/

//example:

/*
jw@jw-laptop ~/CS160Project/vproc/fs $ make
g++ -std=c++11 -Wall -Wextra -DHOME="/home/jw" -O1 -s fs.cpp -o fs.out
jw@jw-laptop ~/CS160Project/vproc/fs $ ./fs.out 777
making file with video id: 777
Opening "wb":
  [/home/jw/Sites/cs160/vdata/vdata_777.dat]
file filled and closed successfully
jw@jw-laptop ~/CS160Project/vproc/fs $ ./fs.out
enter a file to inspect: /home/jw/Sites/cs160/vdata/vdata_777.dat
looking for file:
  [/home/jw/Sites/cs160/vdata/vdata_777.dat]
nframes: 5,
fps: 30,
w: 400,
h: 300
enter a valid 0-indexed frame to inspect, i for next sequential, or q to break
>> i
200,200   0,  0   0,  0   0,  0   0,  0   0,  0   0,  0   0,  0
  0,  0   0,  0   0,  0   0,  0   0,  0   0,  0   0,  0   0,  0
  0,  0   0,  0   0,  0   0,  0   0,  0   0,  0   0,  0   0,  0
  0,  0   0,  0   0,  0   0,  0   0,  0   0,  0   0,  0   0,  0
  0,  0   0,  0   0,  0   0,  0   0,  0   0,  0   0,  0   0,  0
  0,  0   0,  0   0,  0   0,  0   0,  0   0,  0   0,  0   0,  0
  0,  0   0,  0   0,  0   0,  0   0,  0   0,  0   0,  0   0,  0
  0,  0   0,  0   0,  0   0,  0   0,  0   0,  0   0,  0   0,  0
  0,  0   0,  0   0,  0   0,  0
   ... are the (x,y) 68 landmark coords.
left:  { -1,  -1}
right: {  5,   3}
y,p,r: {0.250000, 0.500000, 0.750000}
>> i
201,201   0,  0   0,  0   0,  0   0,  0   0,  0   0,  0   0,  0
  0,  0   0,  0   0,  0   0,  0   0,  0   0,  0   0,  0   0,  0
  0,  0   0,  0   0,  0   0,  0   0,  0   0,  0   0,  0   0,  0
  0,  0   0,  0   0,  0   0,  0   0,  0   0,  0   0,  0   0,  0
  0,  0   0,  0   0,  0   0,  0   0,  0   0,  0   0,  0   0,  0
  0,  0   0,  0   0,  0   0,  0   0,  0   0,  0   0,  0   0,  0
  0,  0   0,  0   0,  0   0,  0   0,  0   0,  0   0,  0   0,  0
  0,  0   0,  0   0,  0   0,  0   0,  0   0,  0   0,  0   0,  0
  0,  0   0,  0   0,  0   0,  0
   ... are the (x,y) 68 landmark coords.
left:  { -1,  -1}
right: {  5,   3}
y,p,r: {0.250000, 0.500000, 0.750000}
>> 4
204,204   0,  0   0,  0   0,  0   0,  0   0,  0   0,  0   0,  0
  0,  0   0,  0   0,  0   0,  0   0,  0   0,  0   0,  0   0,  0
  0,  0   0,  0   0,  0   0,  0   0,  0   0,  0   0,  0   0,  0
  0,  0   0,  0   0,  0   0,  0   0,  0   0,  0   0,  0   0,  0
  0,  0   0,  0   0,  0   0,  0   0,  0   0,  0   0,  0   0,  0
  0,  0   0,  0   0,  0   0,  0   0,  0   0,  0   0,  0   0,  0
  0,  0   0,  0   0,  0   0,  0   0,  0   0,  0   0,  0   0,  0
  0,  0   0,  0   0,  0   0,  0   0,  0   0,  0   0,  0   0,  0
  0,  0   0,  0   0,  0   0,  0
   ... are the (x,y) 68 landmark coords.
left:  { -1,  -1}
right: {  5,   3}
y,p,r: {0.250000, 0.500000, 0.750000}
>> q
enter a file to inspect: ^Z
[12]+  Stopped                 ./fs.out

*/

//have fail count instead of screwy --i thing in deliver/main
#include "fs.h"

#include <stdlib.h>
#include <ctype.h>
#include <string.h>

template<class T>
bool fread_struct(T* p, FILE* fp)
{
    return fread(p, sizeof(T), 1, fp)==1;
}

void print_row(const PairInt16* p, const PairInt16* pend)
{
    for (;; putchar(' '))
    {

        printf("%3d,%3d", p->x(), p->y());
        if (++p==pend)
            break;
    }
    putchar('\n');
}

void print_frame(const FrameResults& all)
{
    const PairInt16 *p64 = all.marks68 + 64, *p = all.marks68;
    for ( ; p!=p64; p+=8)
        print_row(p, p+8);

    print_row(p, p+4);
    puts("   ... are the (x,y) 68 landmark coords.");
    printf("left:  {%3d, %3d}\n"
           "right: {%3d, %3d}\n"
           "y,p,r: {%f, %f, %f}\n",
           all.left_pupil.x(), all.left_pupil.y(),
           all.right_pupil.x(), all.right_pupil.y(),
           all.rotation.yaw, all.rotation.pitch, all.rotation.roll);
}

int main(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    if (argc == 2)
    {
        int videoid = atoi(argv[1]);
        printf("making file with video id: %d\n", videoid);

        VideoMetadata vdata = {
            .nframes = 999,
            .fps = 30,
            .width = 400,
            .height = 300
        };
        FrameResults all={};//zero
        all.left_pupil = {-1, -1};
        all.right_pupil = {5, 3};
        all.rotation = {0.25f, 0.5f, 0.75f};

        VideoDataFiler db;
        db.begin_transaction_ok(videoid);
        db.write_header_ok(vdata);
        for (int i=0; i<5; ++i)
        {
            all.marks68[0] = { int16_t(i + 200), int16_t(i + 200) };
            db.output_frame(all);
        }
        vdata.nframes = 5;//pretend
        db.fixup_header_ok(vdata);
        db.end_transaction_ok();
        return 0;
    }

    char inbuf[256];
    char absolbuf[1u<<13];
    (void)absolbuf;

    VideoMetadata vdata;
    FrameResults all;
    enum{HLEN=sizeof(VideoMetadata)};
    while (fputs("enter a file to inspect: ", stdout), scanf("%256s", inbuf)==1)
    {
        /*
        const char *const pabsol = realpath(inbuf, absolbuf);
        if (!pabsol)
        {
            perror("realpath()");
            continue;
        }
        */
        const char *const pabsol = inbuf;
        printf("looking for file:\n  [%s]\n", pabsol);
        FILE *const fp = fopen(pabsol, "rb");
        if (!fp)
        {
            perror("fopen()");
            continue;
        }
        int nexti=0;
        if (!fread_struct(&vdata, fp))
        {
            perror("fread(&video_header_data)");
            continue;
        }
        printf("nframes: %d,\nfps: %d,\nw: %d,\nh: %d\n", vdata.nframes, vdata.fps, vdata.width, vdata.height);
        puts("enter a valid 0-indexed frame to inspect, i for next sequential, or q to break");
        while ((fputs(">> ",stdout), scanf("%256s", inbuf))==1 && tolower(inbuf[0]) !='q')//tolower
        {
            int reqi;
            if (tolower(inbuf[0])=='i')
                reqi = nexti;
            else
            {
                sscanf(inbuf, "%d", &reqi);

                if (unsigned(reqi) >= unsigned(vdata.nframes))
                {
                    printf("invalid index [%d]\n", reqi);
                    return -1;
                }
                if (fseek(fp, HLEN + reqi*sizeof(FrameResults), SEEK_SET)!=0)
                    perror("fseek");
            }

            if (!fread_struct(&all, fp))
            {
                perror("fread(&all)");
                break;
            }

            print_frame(all);

            if ((nexti = reqi+1) == vdata.nframes)
            {
                nexti = 0;
                if (fseek(fp, HLEN, SEEK_SET)!=0)
                    perror("fseek");
            }
        }//for each frame to look at

        fclose(fp);
    }//for each file to inspect

    return 0;
}

/*

Hey @seanschlaefli, the server stores processed videos in some directory, and the video db table stores a path name, correct? What directory are they stored in? I ask because I would like to have a place to store a file containing all the data related to a video. Bruce said can store that in the filesystem directly. Learning that psql stuff was useful though.
I really should have gotten clear at the requirement gathering stage... that can be something to talk about in the presentation.
seanschlaefli [4:53 PM]
the files are stored in the web directory on the server so it would be something like
/Users/yourusername/Sites/cs160/uploads
on mac at leastg
the path will be different for a linux file system
jonathan [4:55 PM]
So if I want to store file `data.dat` that corresponds to video id `5`, what would the absolute name be of `data.dat`?
seanschlaefli [4:55 PM]
I suppose you could do /Users/seanschlaefli/Sites/cs160/data/5.data.dat
I wouldn't want the data in the uploads directory
but we can create a separate dir for that

*/
