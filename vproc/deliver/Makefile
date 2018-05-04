#example usage, same as when making the library, except you may have to specify where to look to resolve references to dlib:
#   $ make MFLAGS='-mavx2 -I /home/jw/dlib-19.9 -L /home/jw/dlib-19.9/build/dlib'
#MFLAGS can be composed of the following:
#   -I <dlib header dir>  # if not in default path, set this so #include<dlib/xxx.h> will work
#   -L <libdlib.a dir>
#   -mavx2                # if your cpu supports it, if you supply this, also supply it when running the makefile in the ../deliver dir
ifndef MFLAGS
    MFLAGS = 
endif

$(info MFLAGS=$(MFLAGS))

all : main.o db.o hume.o
		g++ $(MFLAGS) -s -o vidproc.out ../library/dlibface.o *.o -ldlib `pkg-config --libs opencv` -lpq

main.o : main.cpp
		g++ $(MFLAGS) -c -O2 -s -std=c++11 -Wall -Wextra -Winvalid-pch -include "../library/pch.h" main.cpp

db.o : db.cpp
		g++ $(MFLAGS) -c -O2 -s -std=c++11 -Wall -Wextra db.cpp

hume.o : hume.cpp
		g++ $(MFLAGS) -c -O2 -s -std=c++11 -Wall -Wextra hume.cpp

