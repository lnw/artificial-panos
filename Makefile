
#CXX=g++-7
CXX=clang++-4.0
AR=ar

GD_INCLUDES_L=-lgd -lpng -lz -lfreetype -lm 
#BOOST_INCLUDES_L=-lboost_regex -lboost_program_options
XML_INCLUDES_L=$(shell pkg-config libxml++-2.6 --libs)
XML_INCLUDES_C=$(shell pkg-config libxml++-2.6 --cflags)
PYTHON_INCLUDES_C=-I/usr/include/python3.5m

HEADERS=array2D.hh auxiliary.hh canvas.hh geometry.hh labelgroup.hh mapitems.hh scene.hh tile.hh

OBJECTS=mapitems.o geometry.o canvas.o tile.o labelgroup.o
OBJECTS_STANDALONE=pano.o
OBJECTS_LIB=interface.o

OBJECTS_P=$(patsubst %.o, build/%.o, $(OBJECTS))
OBJECTS_STANDALONE_P=$(patsubst %.o, build/%.o, $(OBJECTS_STANDALONE))
OBJECTS_LIB_P=$(patsubst %.o, build/%.o, $(OBJECTS_LIB))

build/%.o: %.cc $(HEADERS) Makefile
	$(CXX) -g -O2 -Wshadow -std=c++14 -fpic $(XML_INCLUDES_C) $(PYTHON_INCLUDES_C) -c $< -o $@

pano: $(OBJECTS_P) $(OBJECTS_STANDALONE_P) Makefile
	c++ $(OBJECTS_P) $(OBJECTS_STANDALONE_P) $(GD_INCLUDES_L) $(XML_INCLUDES_L) -o $@

# pano-debug: Makefile pano.cc mapitems.cc geometry.cc canvas.cc $(HEADERS)
# 	$(CXX) -g -O0 -DGRAPHICS_DEBUG -Wall -Wpedantic -Wextra -Wshadow -std=c++14 pano.cc mapitems.cc geometry.cc canvas.cc tile.cc $(GD_INCLUDES_L) $(XML_INCLUDES) -o pano-debug

libartpano.so: $(OBJECTS_P) $(OBJECTS_LIB_P) Makefile
	c++ -shared $(OBJECTS_P) $(OBJECTS_LIB_P) $(GD_INCLUDES_L) $(XML_INCLUDES_L) -o $@


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



.PHONY: testing-invocation-zh testing-invocation-hd testing-invocation-sn
testing-invocation-zh:
	./artpano.py --lat 47.3664 --lon 8.5413  --view-dir-h 290 --view-width 60 --view-height 12 --view-dir-v 4 --range 100000
testing-invocation-hd:
	./artpano.py --lat 49.4 --lon 8.6 --view-dir-h 50 --view-width 355 --view-height 15 --source view1 srtm1 view3 srtm3 --range 40000
testing-invocation-sn:
	./artpano.py --lat 58.2477 --lon 6.5597 --canvas-height 2000 --canvas-width 10000 --view-dir-h 280 --view-width 280 --view-height 40 --range=30000 


