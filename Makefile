#CXX=g++-7
CXX=clang++-4.0

GD_INCLUDES=-lgd -lpng -lz -lfreetype -lm 
BOOST_INCLUDES=-lboost_regex -lboost_program_options
XML_INCLUDES=$(shell pkg-config libxml++-2.6 --cflags --libs)
PYTHON_INCLUDES=-I/usr/include/python3.5m

pano: Makefile pano.cc array2D.hh auxiliary.hh canvas.hh geometry.hh labelgroup.hh mapitems.hh scene.hh tile.hh
	$(CXX) -g -O2 -Wshadow -std=c++14 pano.cc $(GD_INCLUDES) $(XML_INCLUDES) -o pano

pano-debug: Makefile pano.cc array2D.hh auxiliary.hh canvas.hh geometry.hh labelgroup.hh mapitems.hh scene.hh tile.hh
	$(CXX) -g -O0 -DGRAPHICS_DEBUG -Wall -Wpedantic -Wextra -Wshadow -std=c++14 pano.cc $(GD_INCLUDES) $(XML_INCLUDES) -o pano-debug

libartpano.so: Makefile interface.cc array2D.hh auxiliary.hh canvas.hh geometry.hh labelgroup.hh mapitems.hh scene.hh tile.hh
	$(CXX) -g -O2 -Wshadow -std=c++14 interface.cc $(PYTHON_INCLUDES) $(GD_INCLUDES) $(XML_INCLUDES) -shared -fpic -o libartpano.so

.PHONY: test
test: Makefile test.cc scene.hh tile.hh array2D.hh auxiliary.hh
	$(CXX) -g -O2 -std=c++14 test.cc $(XML_INCLUDES) -o test
	./test

# .PHONY: test-circ
# test-circ: Makefile test-circ.cc circ_360.hh
# 	$(CXX) -g -O2 -std=c++14 test-circ.cc -o test-circ
# 	./test-circ

.PHONY: clean distclean
clean:
	rm -f out.png debug*

distclean: clean
	rm -f pano pano-debug test a.out libartpano.so
