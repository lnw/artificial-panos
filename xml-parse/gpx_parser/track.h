#ifndef __TRACK_H__
#define __TRACK_H__

#ifndef PI
#define PI 3.141592654
#endif

#include "tracksegment.h"


class track{
	std::vector<tracksegment> segmentList; //track is a vector of segments

public:
	track& append_segment(); 
	
//	std::vector<tracksegment>::iterator begin(); //returns pointer to first segment
//	std::vector<tracksegment>::iterator end(); //returns pointer to one after last segment
	std::vector<tracksegment>::const_iterator begin() const; //returns pointer to first segment
	std::vector<tracksegment>::const_iterator end() const; //returns pointer to one after last segment
	tracksegment& front(); //returns first segment
	tracksegment& back(); //returns last segment
//	const tracksegment& front() const; //returns first segment
//	const tracksegment& back() const; //returns last segment

	int size() const;

	double ascent() const;
	double descent() const;
	double lateral_distance() const;

	track& gaussian_smooth(std::string choice, float sigma);
	track& remove_clutter(int range, float cutoff);
	track& remove_short_segs(int min_points);

	track& null_times();
	track& set_times(long unsigned int first_time);

	track& crop(float area_lon_min, float area_lat_min, float area_lon_max, float area_lat_max);
};

#endif // __TRACK_H__
