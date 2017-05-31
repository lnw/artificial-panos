
PNG_INCLUDES=-lz -lpng
GD_INCLUDES=-lgd -lpng -lz -lfreetype -lm 
BOOST_INCLUDES=-lboost_regex -lboost_program_options
XML_INCLUDES=$(shell pkg-config libxml++-2.6 --cflags --libs)


pano: Makefile pano.cc canvas.hh scene.hh tile.hh array2D.hh geometry.hh auxiliary.hh colour.hh mapitems.hh
	clang++-3.9 -g -O2 -Wshadow -std=c++14 pano.cc $(GD_INCLUDES) $(XML_INCLUDES) -o pano

pano-debug: Makefile pano.cc canvas.hh scene.hh tile.hh array2D.hh geometry.hh auxiliary.hh colour.hh
	clang++-3.9 -g -O0 -Wall -Wpedantic -Wextra -Wshadow -std=c++14 pano.cc $(GD_INCLUDES) $(XML_INCLUDES) -o pano

.PHONY: test
test: Makefile test.cc scene.hh tile.hh array2D.hh auxiliary.hh
	clang++-3.9 -g -O2 -std=c++14 test.cc $(XML_INCLUDES) -o test
	./test

.PHONY: clean distclean
clean:
	rm -f out.png debug*

distclean: clean
	rm -f pano test a.out
