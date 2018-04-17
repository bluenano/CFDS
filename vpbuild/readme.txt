First, build the precompiled header and the dlibface.o file in ../library.

You must download and unzip shape_predictor_68_face_landmarks.dat
and #DEFINE TRAINED_DATA_FILE to its absolute path.
For example, if it is in
    /home/jw/Data/shape_predictor_68_face_landmarks.dat
Then compile with:
    -DTRAINED_DATA_FILE=\"/home/jw/Data/shape_predictor_68_face_landmarks.dat\"
Note the escapes at the begin/end.

Then to compile and link:

g++ -std=c++11 -O2 -Wall -Wextra -Winvalid-pch -include "../library/pch.h" -DTRAINED_DATA_FILE=\"/home/jw/Data/shape_predictor_68_face_landmarks.dat\" *.cpp -o vprocess.out -s `pkg-config --libs opencv` ../library/dlibface.o -l /path/to/dlib.a

path = ~/Developer/dlib/examples/build/dlib_build/libdlib.a
