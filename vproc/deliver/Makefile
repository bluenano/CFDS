#example usage, same as when making the library, except you may have to specify where to look to resolve references to dlib:
#   $ make MFLAGS='-mavx2 -I /home/jw/dlib-19.9 -L /home/jw/dlib-19.9/build/dlib -DABSOL="/home/jw/CS160Project/data/shape_predictor_68_face_landmarks.dat"'
#MFLAGS can be composed of the following:
#   -I <dlib header dir>  # if not in default path, set this so #include<dlib/xxx.h> will work
#   -L <libdlib.a dir>
#   -mavx2                # if your cpu supports it, if you supply this, also supply it when running the makefile in the ../deliver dir
#MANDATORY:
#   -DABSOL="/path/to/shape_pred.dat"
ifndef MFLAGS
    MFLAGS = 
endif

$(info MFLAGS=$(MFLAGS))

all : main.o db.o hume.o ../library/dlibface.o
		g++ $(MFLAGS) -s -o ../../bin/vidproc.out ../library/dlibface.o *.o -ldlib `pkg-config --libs opencv` -lpq

main.o : main.cpp
		g++ $(MFLAGS) -c -O2 -s -std=c++11 -Wall -Wextra -Winvalid-pch -include "../library/pch.h" `pkg-config --cflags libpq` main.cpp

db.o : db.cpp
		g++ $(MFLAGS) -c -O2 -s -std=c++11 -Wall -Wextra `pkg-config --cflags libpq` db.cpp

hume.o : hume.cpp
		g++ $(MFLAGS) -c -O2 -s -std=c++11 -Wall -Wextra hume.cpp

../library/dlibface.o : ../library/pch.h.gch
		g++ $(MFLAGS) -O2 -s -std=c++11 -Wall -Wextra -Winvalid-pch -c ../library/dlibface.cpp -o ../library/dlibface.o

../library/pch.h.gch : ../library/pch.h
		g++ $(MFLAGS) -O2 -s -std=c++11 -Wall -Wextra -Winvalid-pch -c ../library/pch.h -o ../library/pch.h.gch

#make MFLAGS='-mavx2 -DABSOL="/home/jw/CS160Project/data/shape_predictor_68_face_landmarks.dat"'

