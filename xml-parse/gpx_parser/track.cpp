#include <cstdlib>
#include <vector>
#include <math.h>
#include <iostream>
#include <iomanip> //required for setfill()

#include "track.h"
#include "tracksegment.h"


track& track::append_segment()
{
	tracksegment t;
	segmentList.push_back(t);
	return *this;
}

#if 0
std::vector<tracksegment>::iterator track::begin()
{ return segmentList.begin();}

std::vector<tracksegment>::iterator track::end()
{ return segmentList.end();}
#endif//0

std::vector<tracksegment>::const_iterator track::begin() const
{ return segmentList.begin();}

std::vector<tracksegment>::const_iterator track::end() const
{ return segmentList.end();}

//#if 0
tracksegment& track::front()
{ return segmentList.front();}

tracksegment& track::back()
{ return segmentList.back();}
//#endif //0
#if 0
const tracksegment& track::front() const
{ return segmentList.front();}

const tracksegment& track::back() const
{ return segmentList.back();}
#endif //0

int track::size() const
{
	return segmentList.size();
}


double track::ascent() const
{
	double result(0.0);
	for (std::vector<tracksegment>::const_iterator i = segmentList.begin(); i < segmentList.end(); ++i)
	{
		result += i->tracksegment::ascent();
	}
	return result;
}


double track::descent() const
{
	double result = 0.0;
	for (std::vector<tracksegment>::const_iterator i = segmentList.begin(); i < segmentList.end(); ++i)
	{
		result += i->tracksegment::descent();
	}
	return result;
}


double track::lateral_distance() const
{
	double result = 0.0;
	for (std::vector<tracksegment>::const_iterator i = segmentList.begin(); i < segmentList.end(); ++i)
	{
		result += i->tracksegment::lateral_distance();
	}
	return result;
}


track& track::gaussian_smooth(std::string choice, float sigma)
{
	if(choice == "xy" || choice == "yx")
	{std::cout << "Smoothing lat-list and lon-list (sigma = " << sigma << ") ..." << std::endl;}
	else
	{std::cout << "Smoothing " << choice << "-list (sigma = " << sigma << ") ..." << std::endl;}
	for (std::vector<tracksegment>::iterator i = segmentList.begin(); i < segmentList.end(); ++i)
	{
		i->tracksegment::gaussian_smooth(choice, sigma);
	}
	return *this;
}


track& track::remove_clutter(int range, float cutoff)
{
	std::cout << "Finding points with low correlation to their neighbors (range = " << range << ", cutoff = " << cutoff << ") ..." << std::endl;
	for (std::vector<tracksegment>::iterator i = segmentList.begin(); i < segmentList.end(); ++i)
	{
		i->tracksegment::remove_clutter(range, cutoff);
	}
	return *this;
}


track& track::remove_short_segs(int min_points)
{
	std::cout << "Deleting tracksegments of less than " << min_points << " points ..." << std::endl;
	for (std::vector<tracksegment>::iterator i = segmentList.begin(); i < segmentList.end(); ++i)
	{
		if(i->size() < min_points)
		{
			segmentList.erase(i);
			--i; //because one element was deleted
		}
	}
	return *this;
}

track& track::null_times()
{
	std::cout << "Setting all times to 0 (seconds since 1970) ..." << std::endl;
	for (std::vector<tracksegment>::iterator i = segmentList.begin(); i < segmentList.end(); ++i)
	{
		i->null_times();
	}
	return *this;
}

track& track::set_times(long unsigned int first_time)
{
	//find offset
	long unsigned int offset = *segmentList.begin()->time_begin() - first_time;
	std::cout << "Shifting times of points by " << offset << "s (time of first point is " << first_time << "s) ..." << std::endl;
	//subtract offset
	for (std::vector<tracksegment>::iterator i = segmentList.begin(); i < segmentList.end(); ++i)
	{
		i->set_times(offset);
	}
	return *this;
}

track& track::crop(float area_lon_min, float area_lat_min, float area_lon_max, float area_lat_max)
{
	std::cout << "Discarding points outside of the area " << area_lon_min << " : " << area_lat_min << " : " << area_lon_max << " : " << area_lat_max << " ..." << std::endl; 
	for (std::vector<tracksegment>::iterator i = segmentList.begin(); i < segmentList.end(); ++i)
	{
		i->crop(area_lon_min, area_lat_min, area_lon_max, area_lat_max);
	}
	return *this;
}


