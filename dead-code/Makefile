
CXX=g++
# CXX=clang++ # requires somthing like libomp-dev, which gcc does not require
AR=ar

# FLAGS=-O0 -g -Wshadow -Wunused -std=c++14 -fpic -fopenmp -DNDEBUG
FLAGS=-O3 -g -Wshadow -Wunused -std=c++14 -fpic -fopenmp
LINK_FLAGS=-fopenmp

GD_INCLUDES_L=-lgd -lpng -lz -lfreetype -lm
XML_INCLUDES_L=$(shell pkg-config libxml++-2.6 --libs)
XML_INCLUDES_C=$(shell pkg-config libxml++-2.6 --cflags)
PYTHON_INCLUDES_C=-I/usr/include/python3.7m

HEADERS=array2D.hh auxiliary.hh canvas.hh geometry.hh labelgroup.hh mapitems.hh scene.hh tile.hh

OBJECTS=mapitems.o geometry.o canvas.o tile.o labelgroup.o scene.o
OBJECTS_STANDALONE=pano.o
OBJECTS_LIB=interface.o

OBJECTS_P=$(patsubst %.o, build/%.o, $(OBJECTS))
OBJECTS_STANDALONE_P=$(patsubst %.o, build/%.o, $(OBJECTS_STANDALONE))
OBJECTS_LIB_P=$(patsubst %.o, build/%.o, $(OBJECTS_LIB))

build/%.o: %.cc $(HEADERS) Makefile
	#$(CXX) -DGRAPHICS_DEBUG $(FLAGS) $(XML_INCLUDES_C) $(PYTHON_INCLUDES_C) -c $< -o $@
	$(CXX) $(FLAGS) $(XML_INCLUDES_C) $(PYTHON_INCLUDES_C) -c $< -o $@

pano: $(OBJECTS_P) $(OBJECTS_STANDALONE_P) Makefile
	$(CXX) $(LINK_FLAGS) $(OBJECTS_P) $(OBJECTS_STANDALONE_P) $(GD_INCLUDES_L) $(XML_INCLUDES_L) -o $@

libartpano.so: $(OBJECTS_P) $(OBJECTS_LIB_P) Makefile
	$(CXX) $(LINK_FLAGS) -shared $(OBJECTS_P) $(OBJECTS_LIB_P) $(GD_INCLUDES_L) $(XML_INCLUDES_L) -o $@


.PHONY: test
test: Makefile test.cc scene.hh tile.hh array2D.hh auxiliary.hh
	$(CXX) $(FLAGS) test.cc $(XML_INCLUDES) -o test
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
	./artpano.py --lat 47.3664 --lon 8.5413 --view-dir-h 290 --view-width 60 --view-height 12 --view-dir-v 4 --range 90000 --output out-zh.png
testing-invocation-hd:
	./artpano.py --lat 49.4 --lon 8.6 --view-dir-h 50 --view-width 355 --view-height 15 --source view1 srtm1 view3 srtm3 --range 50000 --output out-hd.png
testing-invocation-sn:
	./artpano.py --lat 58.2477 --lon 6.5597 --canvas-height 2000 --canvas-width 10000 --view-dir-h 280 --view-width 360 --view-height 40 --source view1 --range=40000 --output out-sn.png
testing-invocation-tv:
	./artpano.py --lat 59.95887 --lon 10.80331 --canvas-height 2000 --canvas-width 10000 --view-dir-h 200 --view-width 180 --view-height 30 --range=20000 --source view1 srtm1 view3 srtm3 --output out-tv.png

valgrind-tv:
	valgrind --suppressions=supp --trace-children=yes --leak-check=full --show-reachable=yes --error-limit=no --gen-suppressions=all --log-file=foo.log ./artpano.py --lat 59.95887 --lon 10.80331 --canvas-height 2000 --canvas-width 10000 --view-dir-h 200 --view-width 180 --view-height 30 --range=2000


