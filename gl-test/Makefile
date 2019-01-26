
CXX=g++
#CXX=clang++
AR=ar

FLAGS=-O2 -Wshadow -Wunused -std=c++14 -fpic

GL_INCLUDES_L=-lglut -lGL

triangle: cube.cc Makefile
	$(CXX) triangle.cc $(GL_INCLUDES_L) -o triangle
cube: cube.cc Makefile
	$(CXX) cube.cc $(GL_INCLUDES_L) -o cube



.PHONY: clean distclean

distclean: clean
	rm -f triangle cube



