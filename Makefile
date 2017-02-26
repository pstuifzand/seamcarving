CXX=gcc-6
CXXFLAGS=-std=c++17 -Wno-narrowing -Wall -fconcepts -g `sdl2-config --cflags` -O2
LIBS=-lstdc++ `sdl2-config --libs` -lm

.cpp.o:
	$(CXX) -c -o $@ $< $(CXXFLAGS)

image: image.o
	$(CXX) -o $@ $^ $(LIBS)

image.o: image.cpp tbuffer.hpp math.hpp

test: test.o
	$(CXX) -o $@ $^ $(LIBS)
