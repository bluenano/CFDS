Some steps may be skipped if opencv is installed or the static library libdlib.a was already made.
Also, some things may need a bit of tweaking for your system. There are many ways to link what is needed.

1: 	(skip if have opencv installed)
	Please install ffmpeg and then opencv with options to support more video codecs if you can.
	I followed this guide to install opencv 3.3: http://www.techgazet.com/install-opencv/

		sudo apt-get -y install libopencv-dev build-essential cmake git libgtk2.0-dev
		pkg-config python-dev python-numpy libdc1394-22 libdc1394-22-dev libjpeg-dev libpng12-dev
		libjasper-dev libavcodec-dev libavformat-dev libswscale-dev libgstreamer0.10-dev
		libgstreamer-plugins-base0.10-dev libv4l-dev libtbb-dev libqt4-dev libfaac-dev
		libmp3lame-dev libopencore-amrnb-dev libopencore-amrwb-dev libtheora-dev libvorbis-dev
		libxvidcore-dev x264 v4l-utils unzip

		mkdir opencv

		cd opencv

		wget https://github.com/Itseez/opencv/archive/3.3.0.zip -O opencv-3.3.0.zip

		unzip opencv-3.3.0.zip

		cd opencv-3.3.0

		mkdir build

		cd build

		cmake -D CMAKE_BUILD_TYPE=RELEASE -D CMAKE_INSTALL_PREFIX=/usr/local -D WITH_TBB=ON -D WITH_V4L=ON -D WITH_QT=ON -D WITH_OPENGL=ON ..

		make -j $(nproc)

		sudo make install

		sudo /bin/bash -c 'echo "/usr/local/lib" > /etc/ld.so.conf.d/opencv.conf'

		#created needed linking
		sudo ldconfig

	If that was succesful, you should not get errors for the argument `pkg-config --libs opencv` that will be passed to opencv later.

2:	(skip if have dlib.a)
	Please get the dlib source code and build the static library following their instructions
	(before you do though, you can go into /examples/CMakeLists.txt and delete everything below line 139,
	where most of the example targets are. Trimmed contents can look like trimmed_cmakelists.txt in this vidproc directory).
	If you dont already have the source, you can download and unzip this where I have done the trimming:
	https://drive.google.com/open?id=1Jlpra7nUiMjyXjusWIRbMh4Z2o0r_DJA

		Go into the examples folder and type:
		mkdir build; cd build; cmake .. ; cmake --build .
		That will build all the examples. If you have a CPU that supports AVX instructions then turn them on like this:
		mkdir build; cd build; cmake .. -DUSE_AVX_INSTRUCTIONS=1; cmake --build .

	If that was succesful, then you should hava a static library "libdlib.a" in examples/build/dlib_build

3:	dlib's shape_predictor, which detects the 68 marks after a face bounding box has been detected,
	needs to be initialized with trained data. Currently this is done by a file.
	Please download https://drive.google.com/open?id=15oAnTDNuMSenNLOYmFzUvtI565-cCFHb.
	I recommend unzipping this in the directory you plan the final executable to be, thought it is not a requirement.


4:	(Set exports if needed, any that must be done must be issued before shell script)
	* If the shape predictor file will not be in the same directory as the final executable,
	  Then issue:
		export TRAINED_FILE='/home/name/a/b/c/shape_predictor_68_face_landmarks.dat'
	  And replace that string with where it is for you.
	* Opencv headers will probably be in you default include path and findable with #include <opencv/whatever.hpp>
	  If not, make it so #include <opencv/whatever.hpp> works
	* Unless you fully install dlib, I dont think #include <dlib/whatever.hpp> will work off the bat from anywhere.
	  If it doesn't, issue:
	    export DINC='/home/jw/dlib-19.9'
	  And replace that string with where it is for you. Note there isnt another /dlib at the end.
	  #includes in source are prefixed with dlib/

5:	export DLIB_A_DIR to the directory that contains libdlib.a
	example:
		export DLIB_A_DIR='/home/jw/dlib-19.9/examples/build/dlib_build'

6: 	do
		chmod +x dep.sh

7: 	do
		./dep.sh dep
	This will build a precompiled header for opencv and crunch the dlib crap thats needed once into an object file.
	I geuss if you dont plan on modifying main, then could have been done more direct.

8:	do something like the following:
		g++ -O2 -s -std=c++11 -Wall -Winvalid-pch -include "precomp.h" main.cpp hume.cpp dlibstuff.o `pkg-config --libs opencv` -L "/home/jw/dlib-19.9 (2)/examples/build/dlib_build" -ldlib -o vidproc.out
	And get some sleep.

Hooray.



#provide defaults and put in sensible place
#opencv notes, shoudlnt get error from libs --pkg config
#trim dlib cmake, keep only .a
alias DIR_DLIB_A='/home/jw/dlib-19.9/build/dlib'
alias TRAINED_ABSOL='/home/jw/Data/shape_predictor_68_face_landmarks.dat'
g++ -c -O2 -s -std=c++11 -Wall -Wextra -Winvalid-pch pch.h -o pch.h.gch
g++ -c -O2 -s -std=c++11 -Wall -Wextra -Winvalid-pch -include "pch.h" facedet.cpp
g++ -DJONW -O2 -s -std=c++11 -Wall -Winvalid-pch -include "pch.h" main.cpp hume.cpp facedet.o `pkg-config --libs opencv` -L /home/jw/dlib-19.9/build/dlib -ldlib
#do really need -L thing?







First, build the precompiled header and the dlibface.o file in ../library.

You must download and unzip shape_predictor_68_face_landmarks.dat
and #DEFINE TRAINED_DATA_FILE to its absolute path.
For example, if it is in
    /home/jw/Data/shape_predictor_68_face_landmarks.dat
Then compile with:
    -DTRAINED_DATA_FILE=\"/home/jw/Data/shape_predictor_68_face_landmarks.dat\"
Note the escapes at the begin/end.

Then to compile and link:

g++ -std=c++11 -O2 -Wall -Wextra -Winvalid-pch -include "../library/pch.h" -DTRAINED_DATA_FILE=\"/home/jw/Data/shape_predictor_68_face_landmarks.dat\" *.cpp -o vprocess.out -s `pkg-config --libs opencv dlib-1` ../library/dlibface.o



jw@jw-laptop ~/CS160Project/vidproc $ export GOOB='hello'
jw@jw-laptop ~/CS160Project/vidproc $ ./build.sh 
hello
jw@jw-laptop ~/CS160Project/vidproc $ unset GOOB
jw@jw-laptop ~/CS160Project/vidproc $ ./build.sh 
nodice
j
rofl


-l:

