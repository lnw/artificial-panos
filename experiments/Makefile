
PNG_INCLUDES=-lz -lpng
GD_INCLUDES=-lgd -lpng -lz -ljpeg -lfreetype -lm 
BOOST_INCLUDES=-lboost_regex -lboost_program_options
XML_INCLUDES=$(shell pkg-config libxml++-2.6 --cflags --libs)


libgd-test: Makefile libgd-test.cc
	clang++-3.9 -g -O2 -Wshadow -std=c++14 libgd-test.cc $(GD_INCLUDES) $(XML_INCLUDES) -o libgd-test

read-test: Makefile test_read_bin.cc
	clang++-3.9 -g -O2 -Wshadow -std=c++14 test_read_bin.cc $(XML_INCLUDES) -o read-test


.PHONY: clean distclean
clean:
	rm -f out.png debug*

distclean: clean
	rm -f pano test a.out
