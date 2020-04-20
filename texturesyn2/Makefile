CC=gcc
CXX=g++
RM=rm -f

CFLAGS=-g -O3 -Wall  -std=c++11 
LDFLAGS= -lm

CFILES = lib/cycletimer.cpp lib/instrument.cpp vector2.cpp vector3.cpp  image.cpp main.cpp texture.cpp textureutil.cpp
HFILES = lib/cycletimer.h lib/instrument.h vector2.h vector3.h  all.h image.h texture.h

image: $(CFILES) $(HFILES)
	$(CXX) $(CFLAGS) $(LDFLAGS) -o texturesyn $(CFILES)

clean:
	$(RM) -f texturesyn
	$(RM) -rf *.dSYM

