#example usage, same as when making the library, except you may have to specify where to look to resolve references to dlib:
#   $ make MFLAGS='-mavx2 -I /home/jw/dlib-19.9 -L /home/jw/dlib-19.9/build/dlib -DABSOL="/home/jw/CS160Project/data/shape_predictor_68_face_landmarks.dat"'
#MFLAGS can be composed of the following:
#   -I <dlib header dir>  # if not in default path, set this so #include<dlib/xxx.h> will work
#   -L <libdlib.a dir>
#   -mavx2                # if your cpu supports it.
#MANDATORY:
#   -DABSOL="/shape_predictor_68_face_landmarks.dat"
ifndef MFLAGS
    MFLAGS = 
endif

CC=g++ -std=c++11 -Wall -Wextra -DHOME="$(HOME)"

$(info MFLAGS=$(MFLAGS))

all : main.o hume.o ../library/dlibface.o
		$(CC) $(MFLAGS) -s -o ../../bin/vidproc.out ../library/dlibface.o *.o -ldlib `pkg-config --libs opencv`

main.o : main.cpp
		$(CC) $(MFLAGS) -c -O2 -s -Winvalid-pch -include "../library/pch.h" main.cpp

hume.o : hume.cpp
		$(CC) $(MFLAGS) -c -O2 -s hume.cpp

../library/dlibface.o : ../library/pch.h.gch
		$(CC) $(MFLAGS) -c -O2 -s -Winvalid-pch ../library/dlibface.cpp -o ../library/dlibface.o

../library/pch.h.gch : ../library/pch.h
		$(CC) $(MFLAGS) -c -O2 -s -Winvalid-pch ../library/pch.h -o ../library/pch.h.gch

#make MFLAGS='-mavx2 -DABSOL="/home/jw/CS160Project/data/shape_predictor_68_face_landmarks.dat"'

