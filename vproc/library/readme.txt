#how to build this lib once you have all dependencies (opencv installed, libdlib.a)
#   $ make MFLAGS='-mavx2 -I /home/jw/dlib-19.9'
#MFLAGS can be composed of the following:
#   -I <dlib header dir>  # if not in default path, set this so #include<dlib/xxx.h> will work
#   -mavx2                # if your cpu supports it, if you supply this, also supply it when running the makefile in the ../deliver dir



IMPORTANT:
- Install ffmpeg with support for common codecs.
- Then, install opencv with support for common codecs.
  I followed http://www.techgazet.com/install-opencv/


#dlib build notes:


/*
    The shape_predictor_68_face_landmarks.dat file is needed, and can be downloaded online.
    wget http://dlib.net/files/shape_predictor_68_face_landmarks.dat.bz2
    tar xvjf shape_predictor_68_face_landmarks.dat.bz2

    Minimum suggested flags for compiling a project using both opencv and dlib:
    g++ -Wall -Wextra -std=c++11 *.cpp `pkg-config --libs opencv dlib-1`

    For optimization, add -O2 and -s
*/

/*
    Installing dlib, based on: https://www.learnopencv.com/install-dlib-on-ubuntu/

    First, have opencv installed system wide if not already,
    http://www.techgazet.com/install-opencv/

    get release v19.9
    https://github.com/davisking/dlib/releases

    (Optional speedup) go into examples/CMakeLists.txt and comment out some examples on the bottom,
    we probably don't need any. Here are a few that seems most relevant:
    if (false)
        ... original block ...
    else()
        add_gui_example(train_object_detector)
        add_example(train_shape_predictor_ex)
        add_gui_example(object_detector_advanced_ex)
        add_gui_example(object_detector_ex)
        add_gui_example(image_ex)
        add_gui_example(face_detection_ex)
        add_gui_example(face_landmark_detection_ex)
        add_gui_example(fhog_ex)
        add_gui_example(fhog_object_detector_ex)
    endif()

    tar xvf dlib-19.9.tar.gz
    cd dlib-19.9/
    mkdir build
    cd build
    cmake .. -DUSE_AVX_INSTRUCTIONS=1 -DDLIB_PNG_SUPPORT=1 -DDLIB_JPEG_SUPPORT=1
    cmake --build . --config Release
    sudo make install
    sudo ldconfig

    Again, heres how can compile with whole shebang:

    For release mode, add -O2 -s

    g++ -DDLIB_PNG_SUPPORT=1 -DDLIB_JPEG_SUPPORT=1 -Wall -Wextra -std=c++11 -O2 \
    -s `pkg-config --libs opencv dlib-1` -lpthread -lpng -ljpeg -lX11 *.cpp -o program.out
*/
