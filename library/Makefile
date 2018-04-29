#example usage:
#   $ make MFLAGS='-mavx2 -I /home/jw/dlib-19.9'
#MFLAGS can be composed of the following:
#   -I <dlib header dir>  # if not in default path, set this so #include<dlib/xxx.h> will work
#   -mavx2                # if your cpu supports it
#   -DWANT_6_5            # ignore this

ifndef MFLAGS
    MFLAGS = 
endif

$(info MFLAGS=$(MFLAGS))

dlibface.o : pch.h.gch
		g++ $(MFLAGS) -O2 -s -std=c++11 -Wall -Wextra -Winvalid-pch -c dlibface.cpp

pch.h.gch : pch.h
		g++ $(MFLAGS) -O2 -s -std=c++11 -Wall -Wextra -Winvalid-pch -c pch.h -o pch.h.gch



