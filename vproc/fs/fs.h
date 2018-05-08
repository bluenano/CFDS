#ifndef H_FS
#define H_FS
/* make file:
all : tilde.cpp
	g++ -Wall -Wextra -DHOME="$(HOME)" tilde.cpp -o home_sweet_home.out
*/
/* terminal :
jw@jw-laptop ~/Desktop/elix $ make
g++ -Wall -Wextra -DHOME="/home/jw" tilde.cpp -o home_sweet_home.out
jw@jw-laptop ~/Desktop/elix $ ./home_sweet_home.out
/home/jw
jw@jw-laptop ~/Desktop/elix $ echo $HOME
/home/jw
*/


#define STRINGIFY_PRIV(x) #x
#define STRINGIFY(x) STRINGIFY_PRIV(x)

#ifndef VDATA_DIR// ************************************** @sean is [~/Sites/cs160/vdata] ok?
    #ifdef HOME
        #define VDATA_DIR_STR STRINGIFY(HOME) "/Sites/cs160/vdata"
    #else
        #define VDATA_DIR_STR "."
    #endif
#else
    #define VDATA_DIR_STR STRINGIFY(VDATA_DIR_STR)
#endif

#include <stdio.h>
#include <assert.h>
#include <stdint.h>

//nframes in the output video is not final until all computation is done.
//opencv VideoCapture can fail to read a frame in the middle, or anywhere.
//in that case, will fseek to begin and touch up.
struct VideoMetadata
{
    int32_t nframes;
    int32_t fps;
    int32_t width;
    int32_t height;
};
//16 byte header
static_assert(sizeof(VideoMetadata)==16, "");

struct alignas(4) PairInt16
{
    int16_t x16, y16;//2 bytes each

    int16_t x() const {return x16;}//dlib uses these so it can help with refactoring
    int16_t y() const {return y16;}
};

struct EulerAnglesF32
{
    float yaw, pitch, roll;//4 bytes each
};


struct FrameResults//a POD struct
{
    PairInt16 marks68[68];  //if marks68[0].x==-1 then there was no detection,
                            //and nothing else is valid in for this entire object

    PairInt16 left_pupil;   //if .x()==-1 then not found
    PairInt16 right_pupil;  //if .x()==-1 then not found

    EulerAnglesF32 rotation;//if marks68 found, then these will be filled in
};
//this is a 292 byte structure
static_assert(sizeof(FrameResults)==292, "Inconsistent sizeof(FrameResults).");

/*
order of calls:
    begin_tansact
    - write header
    ~~~ loop ~~~
      - write frame
    ---
    - update header (if vidcap error)
    - end_transact
*/

class VideoDataFiler
{
    enum{NAME_CAP=500};

    FILE *fp=nullptr;//made null on error
    char absol[NAME_CAP];//save to pass to remove() if error occurs midway

    template<class T>
    bool write_ref(const T& oref)
    {
        return fwrite(&oref, sizeof(T), 1, fp) == 1;
    }

    void abort_transaction(const char* msg)
    {
        if (msg)
        {
            perror(msg);
            fclose(fp);
            fp = nullptr;
        }

        fprintf(stderr, "Aborting transaction, file:\n [%s]\n", absol);

        if ( ::remove(absol)==0)
            fputs("File removed.", stderr);
        else
            perror("remove()");
    }

public:
    bool begin_transaction_ok(int videoid)
    {
        assert(fp==nullptr);
        assert(videoid >= 0);//sql doesnt like 0 though

        snprintf(absol, NAME_CAP, VDATA_DIR_STR "/vdata_%d.dat", videoid);
        printf("Opening \"wb\":\n  [%s]\n", absol);

        fp = fopen(absol, "wb");

        if (!fp)
        {
            perror("fopen()");
            return false;
        }
        return true;
    }

    bool end_transaction_ok()
    {
        if (!fp)
            return false;
        bool ok(fclose(fp)==0);
        if (!ok) {
            perror("fclose()"),
            abort_transaction(nullptr);
        }
        else
            puts("file filled and closed successfully");
        fp = nullptr;
        return ok;
    }

    bool write_header_ok(const VideoMetadata& vdata)
    {
        if (!fp)
            return false;

        if (!write_ref(vdata))
        {
            abort_transaction("fwrite header");
            return false;
        }
        else
            return true;
    }

    bool fixup_header_ok(const VideoMetadata& vdata)
    {

        if (fseek(fp, 0L, SEEK_SET)!=0 ||
            !write_ref(vdata) )
        {
            abort_transaction("fseek() or fwrite");
            return false;
        }
        return true;
    }

    bool output_frame(const FrameResults& all)
    {
        if (!write_ref(all))
        {
            abort_transaction("fwrite on a frame data");
            return false;
        }

        return true;
    }
};

#endif
