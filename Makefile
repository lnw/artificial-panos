

moebius: Makefile pano.cc
	clang++-3.9 -g -O2 -std=c++14 pano.cc -o pano

.PHONY: test
test: Makefile test.cc
	clang++-3.9 -g -O2 -std=c++14 test.cc -o test
	./test

distclean:
	rm -f pano test a.out
