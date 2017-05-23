.PHONY: all
all: gpx_parser

gpx_parser: gpx_parser.cpp Makefile track.h track.cpp tracksegment.h tracksegment.cpp conversions.h conversions.cpp 
	g++ gpx_parser.cpp track.cpp tracksegment.cpp conversions.cpp -o gpx_parser $(shell pkg-config libxml++-2.6 --cflags --libs) -lboost_regex -lboost_program_options

clean: 
	rm -f gpx_parser
