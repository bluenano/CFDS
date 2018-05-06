# How to build.   
#### If you already have opencv installed or libdlib.a, you can skip much of this.
#### This looks long, but I've pasted instructions for the above from their websites here also.


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

If that was succesful, you should not get errors for the argument `pkg-config --libs opencv` that will be passed to g++ for linking later.

2:	(skip if have dlib.a)
	Please get the dlib source code and build the static library following their instructions
      (before you do though, you can go into its   
	  /examples/CMakeLists.txt and delete everything below line 139,  
	  where most of the example targets are.   
	  Trimmed contents can look like trimmed_cmakelists.txt in this vidproc directory).  
	If you dont already have the source,   
	you can download and unzip them from here where I have also done the trimming:  
	https://drive.google.com/open?id=1Jlpra7nUiMjyXjusWIRbMh4Z2o0r_DJA  
    DLib's instructions are here for convenience:  

		Go into the examples folder and type:
		mkdir build; cd build; cmake .. ; cmake --build .
		That will build all the examples.   
		If you have a CPU that supports AVX instructions then turn them on like this:
		mkdir build; cd build; cmake .. -DUSE_AVX_INSTRUCTIONS=1; cmake --build .

If that was succesful, then you should hava a static library "libdlib.a" in examples/build/dlib_build

3:	dlib's shape_predictor, which detects the 68 marks after a face bounding box has been detected,  
	needs to be initialized with trained data. Currently this is done by a file.  
	Please download https://drive.google.com/open?id=15oAnTDNuMSenNLOYmFzUvtI565-cCFHb.  
	Unzip it in a place so that it can have an absolute path without special characters like `~/`, `./` or `../`  


4:  cd to my deliver/ and run the makefile. You MUST specify -DABSOL="..." in MFLAGS to the makefile
```
#example usage, same as when making the library, except you may have to specify where to look to resolve references to dlib:
#   $ make MFLAGS='-mavx2 -I /home/jw/dlib-19.9 -L /home/jw/dlib-19.9/build/dlib -DABSOL="/home/jw/CS160Project/data/shape_predictor_68_face_landmarks.dat"'
#MFLAGS can be composed of the following:
#   -I <dlib header dir>  # if not in default path, set this so #include<dlib/xxx.h> will work
#   -L <libdlib.a dir>
#   -mavx2                # if your cpu supports it, if you supply this, also supply it when running the makefile in the ../deliver dir
#MANDATORY:
#   -DABSOL="/path/to/shape_pred.dat"
```

6:  Now you should have an exe 'vidproc.out' built. See top of deliver/main.cpp for how to run it with the right arguments.  

One more thing: have a postgres account with password postgres

(Jonathan): I should have fixed the below, in the source it is now `#include <libpq-fe.h>`, and inside the makefile is ``pkg-config --cflags libpq``  
MacOS notes: I needed to change the include for the libpq-fe.h from <postgresql/libpq-fe.h> to just <libpq-fe.h> in db.h and when you compile in step 5 you need to add -I /path_to_libpq_include. On my system, this was in /usr/local/Cellar/libpq/10.3/include