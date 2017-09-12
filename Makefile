
#CXX=g++-7
CXX=clang++-4.0
AR=ar

GD_INCLUDES=-lgd -lpng -lz -lfreetype -lm 
BOOST_INCLUDES=-lboost_regex -lboost_program_options
XML_INCLUDES_L=$(shell pkg-config libxml++-2.6 --libs)
XML_INCLUDES_C=$(shell pkg-config libxml++-2.6 --cflags)
PYTHON_INCLUDES_C=-I/usr/include/python3.5m

HEADERS=array2D.hh auxiliary.hh canvas.hh geometry.hh labelgroup.hh mapitems.hh scene.hh tile.hh

OBJECTS=mapitems.o geometry.o canvas.o tile.o
OBJECTS_STANDALONE=pano.o
OBJECTS_LIB=interface.o

OBJECTS_P=$(patsubst %.o, build/%.o, $(OBJECTS))
OBJECTS_STANDALONE_P=$(patsubst %.o, build/%.o, $(OBJECTS_STANDALONE))
OBJECTS_LIB_P=$(patsubst %.o, build/%.o, $(OBJECTS_LIB))

build/%.o: %.cc $(HEADERS)
	$(CXX) -g -O2 -Wshadow -std=c++14 -fpic $(XML_INCLUDES_C) $(PYTHON_INCLUDES_C) -c $< -o $@

pano: Makefile $(OBJECTS_P) $(OBJECTS_STANDALONE_P)
	c++ $(OBJECTS_P) $(OBJECTS_STANDALONE_P) $(GD_INCLUDES) $(XML_INCLUDES_L) -o $@

# pano-debug: Makefile pano.cc mapitems.cc geometry.cc canvas.cc $(HEADERS)
# 	$(CXX) -g -O0 -DGRAPHICS_DEBUG -Wall -Wpedantic -Wextra -Wshadow -std=c++14 pano.cc mapitems.cc geometry.cc canvas.cc tile.cc $(GD_INCLUDES) $(XML_INCLUDES) -o pano-debug

libartpano.so: $(OBJECTS_P) $(OBJECTS_LIB_P)
	c++ -shared $(OBJECTS_P) $(OBJECTS_LIB_P) $(GD_INCLUDES) $(XML_INCLUDES_L) -o $@


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
	rm -f pano pano-debug test a.out libartpano.so build/*o



.PHONY: testing-envocation-zh testing-envocation-hd
testing-envocation-zh:
	./artpano.py --lat 47.3664 --lon 8.5413  --view-dir-h 290 --view-width 60 --view-height 12 --view-dir-v 4 --range 100000
testing-envocation-hd:
	./artpano.py --lat 49.4 --lon 8.6 --view-dir-h 50 --view-width 355 --view-height 15 --source view1 srtm1 view3 srtm3 --range 40000



