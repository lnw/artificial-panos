

pano: Makefile pano.cc view.hh scene.hh tile.hh array2D.hh geometry.hh auxiliary.hh
	clang++-3.9 -g -O2 -std=c++14 pano.cc -o pano

.PHONY: test
test: Makefile test.cc scene.hh tile.hh array2D.hh auxiliary.hh
	clang++-3.9 -g -O2 -std=c++14 test.cc -o test
	./test

distclean:
	rm -f pano test a.out
