opencv and dlib (especially the formers use of templates) can lead to longer compile times.
I wanted to see if I could improve it by taking what would likely be needed for the project.

I got a big speed increase by forcing instantions of the template types the dlib face ROI and landmark detectors depend on
in a seperate .cpp file. This can be compiled once, and the corresponding .o can be linked.

Also, a precompiled header is used for opencv includes and other dlib includes I didn't want to dance around.

To build the pch, issue:

    g++ -c -O2 -s -std=c++11 -Wall -Wextra -Winvalid-pch pch.h -o pch.h.gch

To compile dlibface.cpp into the .o:

    g++ -c -O2 -s -std=c++11 -Wall -Wextra -Winvalid-pch -include "pch.h" dlibface.cpp

Then to both compile and link a separate program:

    g++ -O2 -s -std=c++11 -Wall -Wextra -Winvalid-pch -include "../library/pch.h" ../library/dlibface.o `pkg-config --libs opencv dlib-1` -lpthread


../library can be a different path depending on where you're working

-O2 and -s aren't mandatory, but the same flags need to be used with the ones used to make the pch.
(If this is about compilation speed, maybe I shouldn't use -O2 all the time?)


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
