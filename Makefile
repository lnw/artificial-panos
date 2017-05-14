

pano: Makefile pano.cc canvas.hh scene.hh tile.hh array2D.hh geometry.hh auxiliary.hh colour.hh
	clang++-3.9 -g -O2 -std=c++14 pano.cc -lz -lpng -o pano

pano-debug: Makefile pano.cc canvas.hh scene.hh tile.hh array2D.hh geometry.hh auxiliary.hh colour.hh
	clang++-3.9 -g -O0 -Wall -Wpedantic -Wextra -std=c++14 pano.cc -lz -lpng -o pano

.PHONY: test
test: Makefile test.cc scene.hh tile.hh array2D.hh auxiliary.hh
	clang++-3.9 -g -O2 -std=c++14 test.cc -o test
	./test

.PHONY: clean distclean
clean:
	rm -f out.png debug*

distclean: clean
	rm -f pano test a.out
