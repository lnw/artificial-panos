

moebius: Makefile pano.cc
	clang++-3.9 -g -O2 -std=c++14 pano.cc -o pano

distclean:
	rm -f pano
