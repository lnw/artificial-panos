#include <cstdlib>
#include <vector>
#include <math.h>
#include <iostream>
#include <iomanip> //required for setfill()

#include "tracksegment.h"

tracksegment& tracksegment::append_point(double lat, double lon, double ele, long unsigned int time)
{
	latitudeList.push_back(lat);
	longitudeList.push_back(lon);
	elevationList.push_back(ele);
	timeList.push_back(time);
	return *this;
}


std::vector<double>::const_iterator tracksegment::lat_begin() const
{return latitudeList.begin();}

std::vector<double>::const_iterator tracksegment::lon_begin() const
{return longitudeList.begin();}

std::vector<double>::const_iterator tracksegment::ele_begin() const
{return elevationList.begin();}

std::vector<long unsigned int>::const_iterator tracksegment::time_begin() const
{return timeList.begin();}

std::vector<double>::const_iterator tracksegment::lat_end() const
{return latitudeList.end();}

std::vector<double>::const_iterator tracksegment::lon_end() const
{return longitudeList.end();}

std::vector<double>::const_iterator tracksegment::ele_end() const
{return elevationList.end();}

std::vector<long unsigned int>::const_iterator tracksegment::time_end() const
{return timeList.end();}

double tracksegment::lat(std::vector<double>::const_iterator lat_iter) const
{return *lat_iter;}

double tracksegment::lon(std::vector<double>::const_iterator lon_iter) const
{return *lon_iter;}

double tracksegment::ele(std::vector<double>::const_iterator ele_iter) const
{return *ele_iter;}

long unsigned int tracksegment::time(std::vector<long unsigned int>::const_iterator time_iter) const
{return *time_iter;}


int tracksegment::size() const
{
	return latitudeList.size();
}


//iterate over latitudeList and longitudeList and add great_circle_distance for each pair
double tracksegment::lateral_distance() const
{
	double result = 0.0;
	double lastLat = latitudeList.front();
	double lastLon = longitudeList.front();
	std::vector<double>::const_iterator latIter = latitudeList.begin() + 1;
	std::vector<double>::const_iterator lonIter = longitudeList.begin() + 1;
	while(latIter < latitudeList.end())
	{
		result += great_circle_distance(lastLat, lastLon, *latIter, *lonIter);
		lastLat = *latIter;
		lastLon = *lonIter;
		++latIter;
		++lonIter;
	}
	return result; // in km
}


//einmal ueber die Liste iterieren, alle Aufstiege werden addiert
double tracksegment::ascent() const
{
	double result = 0.0;
	double lastEl = elevationList.front();
	for(std::vector<double>::const_iterator elIter = elevationList.begin() + 1; elIter < elevationList.end(); ++elIter)
	{
		if(*elIter > lastEl)
		{
			result += (*elIter - lastEl);
		}
		lastEl = *elIter;
	}
	return result; 
}


//einmal ueber die Liste iterieren, alle Abstiege werden addiert
double tracksegment::descent() const
{
	double result = 0.0;
	double lastEl = elevationList.front();
	for(std::vector<double>::const_iterator elIter = elevationList.begin() + 1; elIter < elevationList.end(); ++elIter)
	{
		if(*elIter < lastEl)
		{
			result += (*elIter - lastEl);
		}
		lastEl = *elIter;
	}
	return result;
}


//sinnvolle Werte fuer range und cutoff sind 3 und 1.05
tracksegment& tracksegment::remove_clutter(int range, float cutoff)
{
	//std::cout << "Finding points with low correlation to their neighbors (range = " << range << ", cutoff = " << cutoff << ") ..." << std::endl;
	//vier temporaere Listen
	std::vector<double> new_latList;
	std::vector<double> new_lonList;
	std::vector<double> new_eleList;
	std::vector<unsigned long int> new_tList;
	//vier Iterators für die alten Listen
	std::vector<double>::iterator latIter = latitudeList.begin();
	std::vector<double>::iterator lonIter = longitudeList.begin();
	std::vector<double>::iterator eleIter = elevationList.begin();
	std::vector<unsigned long int>::iterator tIter = timeList.begin();
	for(/* */;latIter < latitudeList.end(); ++latIter, ++lonIter, ++eleIter, ++tIter)
	{
		double direct_distance = 0;
		double path_following_distance = 0;
		double before_sum = 0;
		double after_sum = 0;
		for(int i = -2; i >= -range; --i)
		{
			direct_distance = great_circle_distance(*(latIter+i), *(lonIter+i), *latIter, *lonIter);
			path_following_distance = 0;
			for(int j = -1; j >= i; --j)
			{
				path_following_distance += great_circle_distance(*(latIter+j), *(lonIter+j), *(latIter+j+1), *(lonIter+j+1));
			}
			before_sum += path_following_distance / direct_distance;
			//std::cout << direct_distance << "\t" << path_following_distance << "\t" << before_sum << "\t" << i << std::endl;
		}
		//std::cout << before_sum / range << std::endl;
		for(int i = 2; i<= range; ++i) //2, um das erste Segment auszulassen, weil x/x=1 nicht hilfreich ist
		{
			direct_distance = great_circle_distance(*latIter, *lonIter, *(latIter+i), *(lonIter+i));
			path_following_distance = 0;
			for(int j = 1; j <= i; ++j)
			{
				path_following_distance += great_circle_distance(*(latIter+j-1), *(lonIter+j-1), *(latIter+j), *(lonIter+j));
			}
			after_sum += path_following_distance / direct_distance;
			//std::cout << direct_distance << "\t" << path_following_distance << "\t" << after_sum << "\t" << i << std::endl;
		}
		//std::cout << after_sum / range << std::endl;
		//übertragen der "guten" Punkte in neue Listen
		if(before_sum / (range-1) < cutoff || after_sum / (range-1) < cutoff)
		{
			new_latList.push_back(*latIter);
			new_lonList.push_back(*lonIter);
			new_eleList.push_back(*eleIter);
			new_tList.push_back(*tIter);
		}
	}
	std::cout << "Deleting " << latitudeList.size() - new_latList.size() << " out of " << latitudeList.size() << " points ..." << std::endl;
	//zurückkopieren
	latitudeList = new_latList;
	longitudeList = new_lonList;
	elevationList = new_eleList;
	timeList = new_tList;
	//std::cout << "Done" << std::endl;
	return *this;
}


tracksegment& tracksegment::gaussian_smooth(std::string choice, float sigma)
{
	if(choice == "lat")
	{
		do_gaussian_smooth(&latitudeList, sigma);
		//std::cout << "lat chosen" << std::endl;
	}
	else if(choice == "lon")
	{
		do_gaussian_smooth(&longitudeList, sigma);
		//std::cout << "lon chosen" << std::endl;
	}
	else if(choice == "xy" || choice == "yx")
	{
		do_gaussian_smooth(&latitudeList, sigma);
		do_gaussian_smooth(&longitudeList, sigma);
		//std::cout << "lat+lon chosen" << std::endl;
	}
	else if(choice == "ele" || choice == "z")
	{
		do_gaussian_smooth(&elevationList, sigma);
		//std::cout << "ele chosen" << std::endl;
	}
	else
	{
		std::cout << "Invalid list identifier.  No smoothing was done ..." << std::endl;
	}
	return *this;
}


//smoothing of a list
//arguments are: list, sigma
void tracksegment::do_gaussian_smooth(std::vector<double> *chosen_list, float sigma)
{
	//generieren der Maske
	double sum = 0.0;
	double tmp;
	std::vector<double> mask;
	for(int i=0; sum <0.95; ++i)
	{	
		tmp = gaussian(i,sigma); //funktionswert der Gaussfunktion damit er nicht dreimal berechnet wird
		if(i==0)
		{
			sum += tmp;
			mask.push_back(tmp);
		}
		else
		{
			sum += 2 * tmp;
			mask.insert(mask.begin(), tmp);
			mask.push_back(tmp);
		}
	}
	//normieren der Maske
	for(std::vector<double>::iterator i = mask.begin(); i < mask.end(); ++i)
	{
		*i /= sum;
	}
	//erweitern der zu glaettenden Liste um (mask.size()-1)/2 zu beiden Seiten,
	//und zwar erstmal mit dem ersten bzw letzten wert.
	chosen_list->insert(chosen_list->begin(), (mask.size()-1)/2, chosen_list->front());
	chosen_list->insert(chosen_list->end(), (mask.size()-1)/2, chosen_list->back());
	//neue liste schreiben, gewichtet aus der alten
	//initialisieren mit Nullen, iterieren von "list" und addieren nach "new_list"
	std::vector<double> new_list(chosen_list->size(), 0.0);
	std::vector<double>::iterator i = chosen_list->begin() + (mask.size()-1)/2;
	std::vector<double>::iterator ii = new_list.begin() + (mask.size()-1)/2;
	for(/* */; i < chosen_list->end() - (mask.size()-1)/2; ++i, ++ii)
	{
		int foo = -(mask.size()-1)/2; //ein int der die position in mask angibt. Eigentlich muesste man size_type wählen aber das ist ja das gleiche
		for(std::vector<double>::iterator j = mask.begin(); j < mask.end(); ++j, ++foo)
		{
			*ii += *(i+foo) * *j;
			//*ii += *j;
		}
	}
	//uebertragen von "new_list" nach "list"	
	*chosen_list = new_list;
	//entfernen der ersten und letzten (mask.size()-1)/2 werte
	chosen_list->erase(chosen_list->begin(), chosen_list->begin()+(mask.size()-1)/2);
	chosen_list->erase(chosen_list->end()-(mask.size()-1)/2, chosen_list->end());
	//std::cout << " done" << std::endl;
}


tracksegment& tracksegment::null_times()
{
	for (std::vector<long unsigned int>::iterator i = timeList.begin(); i < timeList.end(); ++i)
	{
		*i = 0;
	}
	return *this;
}


tracksegment& tracksegment::set_times(long unsigned int offset)
{
	for (std::vector<long unsigned int>::iterator i = timeList.begin(); i < timeList.end(); ++i)
	{
		*i -= offset;
	}
	return *this;
}


tracksegment& tracksegment::crop(float area_lon_min, float area_lat_min, float area_lon_max, float area_lat_max)
{
	std::vector<double>::iterator latIter = latitudeList.begin();
	std::vector<double>::iterator lonIter = longitudeList.begin();
	std::vector<double>::iterator eleIter = elevationList.begin();
	std::vector<unsigned long int>::iterator tIter = timeList.begin();
	for(/* */;latIter < latitudeList.end(); ++latIter, ++lonIter, ++eleIter, ++tIter)
	{
		if(*lonIter < area_lon_min || *latIter < area_lat_min || *lonIter > area_lon_max || *latIter > area_lat_max)
		{
			latitudeList.erase(latIter--); //wenn man schon den pointer in der hand hat, kann man auch gleich dekrementieren (wegen loeschen ...)
			longitudeList.erase(lonIter--);
			elevationList.erase(eleIter--);
			timeList.erase(tIter--);
		}
	}
	return *this;
}


//Entfernung von zwei Punkten a und b entlang von Grosskreisen auf einer Kugel.
//lambda sind grad westlicher länge (longitude) und phi grad noerdlicher breite (latitude).
double great_circle_distance(double phi_a, double lambda_a, double phi_b, double lambda_b)
{
	const double earth_radius = 6367.5; //in km //polar: 6357, equatorial: 6378
	double distance_angle = acos( sin(deg2rad(phi_a))*sin(deg2rad(phi_b)) + cos(deg2rad(phi_a))*cos(deg2rad(phi_b))*cos(deg2rad(lambda_a) - deg2rad(lambda_b))); //in radians
	double distance = distance_angle * earth_radius; // in km
	return distance; // in km
}


double gaussian(double x, float sigma)
{return 1/( sigma * sqrt(2 * PI)) * exp(-x*x /(2 * sigma*sigma));}


double deg2rad(double deg)
{return deg * PI / 180.0;}


double rad2deg(double rad)
{return rad * 180.0 / PI;}
