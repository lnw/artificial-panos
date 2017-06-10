#CXX=g++-6
CXX=clang++-3.9

PNG_INCLUDES=-lz -lpng
GD_INCLUDES=-lgd -lpng -lz -lfreetype -lm 
BOOST_INCLUDES=-lboost_regex -lboost_program_options
XML_INCLUDES=$(shell pkg-config libxml++-2.6 --cflags --libs)


pano: Makefile pano.cc array2D.hh  auxiliary.hh  canvas.hh  circ_360.hh  colour.hh  geometry.hh  labelgroup.hh  mapitems.hh  scene.hh  tile.hh
	$(CXX) -g -O2 -Wshadow -std=c++14 pano.cc $(GD_INCLUDES) $(XML_INCLUDES) -o pano

pano-debug: Makefile pano.cc array2D.hh  auxiliary.hh  canvas.hh  circ_360.hh  colour.hh  geometry.hh  labelgroup.hh  mapitems.hh  scene.hh  tile.hh
	$(CXX) -g -O0 -DGRAPHICS_DEBUG -Wall -Wpedantic -Wextra -Wshadow -std=c++14 pano.cc $(GD_INCLUDES) $(XML_INCLUDES) -o pano-debug

.PHONY: test test-circ
test: Makefile test.cc scene.hh tile.hh array2D.hh auxiliary.hh
	clang++-3.9 -g -O2 -std=c++14 test.cc $(XML_INCLUDES) -o test
	./test

test-circ: Makefile test-circ.cc circ_360.hh
	clang++-3.9 -g -O2 -std=c++14 test-circ.cc -o test-circ
	./test-circ

.PHONY: clean distclean
clean:
	rm -f out.png debug*

distclean: clean
	rm -f pano pano-debug test a.out
