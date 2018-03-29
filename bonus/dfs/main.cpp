/*
$ g++ -Winvalid-pch -include pch.h -O2 -std=c++11 -Wextra -Wall *.cpp -o dfs.out -s `pkg-config --libs opencv`

$ ./dfs
found blob of 10 points
(avg x:   8, avg y:   4)

$ ./dfs eye0.png 8
cut: 8
found blob of 2902 points
(avg x: 323, avg y: 140)

    Let lower gray scale numbers indicate points interested in.
    Pupil is black already, so don't do an invert.


    Let CUT(x) cut on the left.
    So if cut is 5, the 5 nonneg numbers strictly < 5 are considered as black.
*/
#include "pch.h"

#include <stdio.h>

struct Result
{
    unsigned npoints, xsum, ysum;
};

void add(Result *r, Result const b)
{
    r->npoints += b.npoints;
    r->xsum    += b.xsum;
    r->ysum    += b.ysum;
}

//note, instead of another visible[], this trashes a[] input by setting to black
//simpler(?) for my use case
//
//returns number of black points at this coord, and adjacent to it,
//marking all those as white
//
//handling out of bounds and other things could maybe be done with less operations
Result Dfs(unsigned char *a, int nrows, int w, unsigned char cut, int i, int j)
{
    Result res = {};//zero'd

    if (0<=i && i<nrows && 0<=j && j<w)
    {
        unsigned char *const ptr = a + i*w + j;
        if (*ptr < cut)
        {
            *ptr = 255;//got this coord, can pretend white from now on to signal not to visit

            res.npoints++;
            res.xsum += j;
            res.ysum += i;

            add(&res, Dfs(a, nrows, w, cut, i+1, j));//down
            add(&res, Dfs(a, nrows, w, cut, i-1, j));//up
            add(&res, Dfs(a, nrows, w, cut, i, j+1));//right
            add(&res, Dfs(a, nrows, w, cut, i, j-1));//left
        }
    }

    return res;
}

enum {DEF_CUT = 8};//needs to be pretty low for some pictures with big lashes
//could do something more like track blob x min and max to compute span, and same for y.
//then see if bounding rect is close to being square, which should be true for pupil, but not for eyelashes
//
//as of now just leave this high, it worked for a pretty fat eyelash

//note, instead of another visible[], this trashes a[] input by setting to black
//simpler(?) for my use case
Result LargestBlob(unsigned char *a, int nrows, int w, unsigned char cut)
{
    Result maxres = {};

    for (int i=0; i!=nrows; ++i)
    {
        for (int j=0; j!=w; ++j)
        {
            if (a[i*w + j] < cut)//if unvisited
            {
                Result const cres = Dfs(a, nrows, w, cut, i, j);
                if (maxres.npoints < cres.npoints)
                    maxres = cres;
            }
        }
    }

    return maxres;
}

enum
{
    NROWS = 7,
    WIDTH = 11,
    END = NROWS*WIDTH
};

int test()
{
    char a[]=
    {
    //...012345678901
        ".A.A.A.A.A."//0
        "..........."//1
        ".AAAA......"//2
        ".AAA.A.AAAA"//3
        "..A....A..A"//4
        "AA...A.AAAA"//5
        ".A........."//6
    };

    static_assert(sizeof(a) == END+1, "");//+1: '\0' not convenient to remove

    for (int i=0; i!=END; ++i)
        a[i] = a[i] == '.' ? 255 : 0;

    Result const r = LargestBlob((unsigned char *)a, NROWS, WIDTH, DEF_CUT);

    if (r.npoints)
    {
        printf("found blob of %u points\n(avg x: %3u, avg y: %3u)\n",
               r.npoints, r.xsum/r.npoints, r.ysum/r.npoints);
    }
    else
        puts("no points found");

    return 0;
}
/*
    //...012345678901
        ".A.A.A.A.A."//0
        "..........."//1
        ".AAAA......"//2
        ".AAA.A.AAAA"//3
        "..A....A..A"//4
        "AA...A.AAAA"//5
        ".A........."//6

found blob of 10 points
(avg x:   8, avg y:   4)

*/

#include <stdlib.h>

int main(int argc, char **argv)
{
    if (argc<2)
        return test();

    unsigned cut = DEF_CUT;
    if (argc==3)
        if ( (cut = atoi(argv[2])) > 255u || cut==0u)
            cut = DEF_CUT;

    printf("cut: %u\n", cut);

    using namespace cv;
    cv::String const wname = "w";
    namedWindow(wname);//no handle or something?

    Mat src = imread(argv[1]);
    Mat gray;
    cvtColor(static_cast<const Mat&>(src), gray, CV_BGR2GRAY);
    //imwrite("grayscale8.png", gray);
    imshow(wname, gray);
    waitKey(0);
    #if 0
    cv::threshold(gray, gray, cut, 255, cv::THRESH_BINARY);
    //imwrite("cut.png", gray);
    imshow(wname, gray);
    waitKey(0);
    #endif//like thought, does not do add anything if compare against cut in dfs, it was just 32 was too big

    if (gray.total()*gray.elemSize() != (unsigned)gray.rows*gray.cols)
    {
        puts("grayscale mat elements not bytes?");
        return 1;
    }

    Result const r = LargestBlob((unsigned char *)gray.data, gray.rows, gray.cols, cut);

    if (r.npoints)
    {
        unsigned const x = r.xsum/r.npoints, y = r.ysum/r.npoints;

        cv::circle(src, Point(x, y), 7, CV_RGB(0,255,0), CV_FILLED);
        //imwrite("dot.png", src);
        imshow(wname, src);
        waitKey(0);

        printf("found blob of %u points\n(avg x: %3u, avg y: %3u)\n",
               r.npoints, x, y);
    }
    else
        puts("no points found");

    return 0;
}


//a reflection spot in the pupil can offset where the pupil center really is
