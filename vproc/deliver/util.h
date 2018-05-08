#ifndef H_UTIL
#define H_UTIL

#ifndef _POSIX_C_SOURCE
    #define _POSIX_C_SOURCE 200809L
#elif (_POSIX_C_SOURCE) < 199309L
    #error compile with -D_POSIX_C_SOURCE=200809L
#endif

#include <time.h>
inline
unsigned long get_millisecs_stamp(void)
{
    struct timespec spec;

    clock_gettime(CLOCK_MONOTONIC, &spec);

    return spec.tv_sec*1000u + spec.tv_nsec/(1000u*1000u);//10e6 millisecs per nanosec
}

inline
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

#endif
