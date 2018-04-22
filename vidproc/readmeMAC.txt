The following guide will describe how to get Jonathan's program
running on Mac OS.

Install OpenCV, I used https://www.learnopencv.com/install-opencv3-on-macos/.
You can skip the parts of the guide that describe setting up OpenCV in a
virtual environment.

This guide might be more helpful:
https://blogs.wcode.org/2014/10/howto-install-build-and-use-opencv-macosx-10-10/

Next, you need to install dlib. This was a little tricky, but basically you
need to install dlib using Homebrew first.

brew install dlib

Then, you need to clone dlib's github repository to somewhere on your system.

git clone https://github.com/davisking/dlib

I cleared the contents of /usr/local/include/dlib and then copied the contents of the github repository to that folder.
BE CAREFUL WHEN DOING THIS

rm -r /usr/local/include/dlib
cp -r PATH_TO_DLIB_FROM_GITHUB /usr/local/include/dlib

When you clone dlib from github the folder you want to copy is ../dlib/dlib/

Instead of doing the above, try cloning the github repository directly
into /usr/local/include.
This might work, but I can't guarantee it.

Now, navigate to the examples directory in dlib and run the following commands:

mkdir build
cd build
cmake ..
cmake --build .

If that was succesful, then you should hava a static library "libdlib.a"
in examples/build/dlib_build

Download dlib's shape_predictor file at:
https://drive.google.com/open?id=15oAnTDNuMSenNLOYmFzUvtI565-cCFHb

Place this file into the directory where the executable will live.
It must be unzipped so run:

bzip2 -d shape_predictor_68_face_landmarks.dat.bz2

If the shape predictor file will not be in the same directory as the final executable,

Then issue:
export TRAINED_FILE='/home/name/a/b/c/shape_predictor_68_face_landmarks.dat'
And replace that string with where it is for you.

Unless you fully install dlib, I dont think #include <dlib/whatever.hpp> will work off the bat from anywhere.
If it doesn't, issue:
export DINC='/home/jw/dlib-19.9'
Note there isnt another dlib because includes are prefixed with dlib in the source

Export DLIB_A_DIR to the directory that contains libdlib.a:
export DLIB_A_DIR='/home/jw/dlib-19.9/examples/build/dlib_build'

Make dep.sh executable:
chmod +x dep.sh

Run:
./dep.sh

Finally, compile the entire program by running:

g++ -O2 -s -std=c++11 -Wall -Winvalid-pch -include "precomp.h" main.cpp hume.cpp dlibstuff.o `pkg-config --libs opencv` -L "PATH_TO_DLIB_BUILD_DIR" -ldlib -o vidproc.out

PATH_TO_DLIB_BUILD_DIR might be something like
/usr/local/include/dlib/dlib/examples/build/dlib_build
