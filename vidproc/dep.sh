#!/bin/sh

# if [ $# -eq 1 ]; then
    echo "building pch and dlibstuff.o"

	if [ -n "$DINC" ]; then
		g++ -I $DINC -c -O2 -s -std=c++11 -Wall -Wextra -Winvalid-pch precomp.h -o precomp.h.gch
		g++ -I $DINC -c -O2 -s -std=c++11 -Wall -Wextra -Winvalid-pch -include "precomp.h" dlibstuff.cpp
	else
		g++ -c -O2 -s -std=c++11 -Wall -Wextra -Winvalid-pch precomp.h -o precomp.h.gch
		g++ -c -O2 -s -std=c++11 -Wall -Wextra -Winvalid-pch -include "precomp.h" dlibstuff.cpp
	fi
# else


