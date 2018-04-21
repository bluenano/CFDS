Some steps may be skipped if opencv is installed or the static library libdlib.a was already made.
Also, some things may need a bit of tweaking for your system. There are many ways to link/include what is needed.

1: 	(skip if have opencv installed)
	Please install ffmpeg and then opencv with options to support more video codecs if you can.
	I followed this guide to install opencv 3.3: http://www.techgazet.com/install-opencv/

		sudo apt-get -y install libopencv-dev build-essential   
		cmake git libgtk2.0-dev
		pkg-config python-dev python-numpy libdc1394-22   
		libdc1394-22-dev libjpeg-dev libpng12-dev
		libjasper-dev libavcodec-dev libavformat-dev   
		libswscale-dev libgstreamer0.10-dev
		libgstreamer-plugins-base0.10-dev libv4l-dev   
		libtbb-dev libqt4-dev libfaac-dev
		libmp3lame-dev libopencore-amrnb-dev libopencore-amrwb-dev   
		libtheora-dev libvorbis-dev
		libxvidcore-dev x264 v4l-utils unzip

		mkdir opencv

		cd opencv

		wget https://github.com/Itseez/opencv/archive/3.3.0.zip -O opencv-3.3.0.zip

		unzip opencv-3.3.0.zip

		cd opencv-3.3.0

		mkdir build

		cd build

		cmake -D CMAKE_BUILD_TYPE=RELEASE -D CMAKE_INSTALL_PREFIX=/usr/local   
		-D WITH_TBB=ON -D WITH_V4L=ON -D WITH_QT=ON -D WITH_OPENGL=ON ..

		make -j $(nproc)

		sudo make install

		sudo /bin/bash -c 'echo "/usr/local/lib" > /etc/ld.so.conf.d/opencv.conf'

		#created needed linking
		sudo ldconfig

	If that was succesful, you should not get errors for the argument `pkg-config --libs opencv` that will be passed to g++ later.

2:	(skip if have dlib.a)
	Please get the dlib source code and build the static library following their instructions (before you do though,  
	you can go into   
	/examples/CMakeLists.txt and delete everything below line 139,
	where most of the example targets are.   
	Trimmed contents can look like trimmed_cmakelists.txt in this vidproc directory).
	If you dont already have the source,   
	you can download and unzip them from here where I have also done the trimming:
	https://drive.google.com/open?id=1Jlpra7nUiMjyXjusWIRbMh4Z2o0r_DJA

		Go into the examples folder and type:
		mkdir build; cd build; cmake .. ; cmake --build .
		That will build all the examples.   
		If you have a CPU that supports AVX instructions then turn them on like this:
		mkdir build; cd build; cmake .. -DUSE_AVX_INSTRUCTIONS=1; cmake --build .

	If that was succesful, then you should hava a static library "libdlib.a" in examples/build/dlib_build

3:	dlib's shape_predictor, which detects the 68 marks after a face bounding box has been detected,
	needs to be initialized with trained data. Currently this is done by a file.
	Please download https://drive.google.com/open?id=15oAnTDNuMSenNLOYmFzUvtI565-cCFHb.
	I recommend unzipping this in the directory you plan the final executable to be, thought it is not a requirement.


6: do
	`chmod +x dep.sh`

7: do
	`./dep.sh dep`
	This will build a precompiled header for opencv and crunch the dlib crap thats needed once into an object file.
	I geuss if you dont plan on modifying main, then this process could have been done more direct.

8:	do something like the following (3 are optional, and if you do use them modify the paths for your system):
	-DTRAINED_FILE=\"...\" # if the trained file will not be in the final executable dir, specify its absolute path,  
		(notice the \" at both ends).
	-I "/home/jw/dlib-19.9" # if you did not do a full install of dlib and the headers are not in your default include path.  
		(note that headers are #included in source code as <dlib/image_processing.h>  
		NOT <image_processing.h>. same for opencv).  
	-L "/home/jw/dlib-19.9/examples/build/dlib_build" # if the static library libdlib.a is not in a place automatically  
		looked for by your linker.  

	`g++ -I "/home/jw/dlib-19.9" -DTRAINED_FILE=\"/home/name/a/b/c/shape_predictor_68_face_landmarks.dat\"  
	-O2 -s -std=c++11 -Wall -Winvalid-pch -include "precomp.h" main.cpp hume.cpp dlibstuff.o `pkg-config --libs opencv` -L "/home/jw/dlib-19.9/examples/build/dlib_build" -ldlib -o vidproc.out`
