#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <cstdlib>
//#include <vector>
#include <math.h>
#include <iostream>
#include <iomanip> //required for setfill()
#include <boost/regex.hpp>
#include <libxml++/libxml++.h> //definiert GLib::ustring (das aus dem xml faellt)

#include "conversions.h"


#if 0
export template <typename T> double to_double(const T& s)
//export is not supperted by gcc
{
std::stringstream ss;
double result;
ss << s;
ss >> result;
return result;
}

export template <typename T> int to_int(const T& s)
{
std::stringstream ss;
int result;
ss << s.raw();
ss >> result;
return result;
}
#endif //0

Glib::ustring double_to_ustring(double d)
{
	std::stringstream s;
	s.precision(12);
	Glib::ustring result;
	s << d;
	s >> result;
	return result;
}


Glib::ustring int_to_ustring(int i)
{
	std::stringstream s;
	s.precision(2);
	Glib::ustring result;
	s << std::setw(2) << std::setfill('0') << i;
	s >> result;
	return result;
}

//reverse modulus
int rev_mod(int i, int j)
{
	return (i - (i%j))/j;
}


unsigned long int ustring_to_sec(Glib::ustring time)
{
boost::regex re("(\\d{4})\\-(\\d{2})\\-(\\d{2})T(\\d{2})\\:(\\d{2})\\:(\\d{2})Z");
boost::cmatch matches;
if(boost::regex_match(time.c_str(), matches, re))
{
Glib::ustring year(matches[1].first, matches[1].second);
Glib::ustring month(matches[2].first, matches[2].second);
Glib::ustring day(matches[3].first, matches[3].second);
Glib::ustring hour(matches[4].first, matches[4].second);
Glib::ustring min(matches[5].first, matches[5].second);
Glib::ustring sec(matches[6].first, matches[6].second);
unsigned int year_i = to_int(year);
unsigned int month_i = to_int(month);
unsigned int day_i = to_int(day);
unsigned long int hour_i = to_int(hour);
unsigned long int min_i = to_int(min);
unsigned long int sec_i = to_int(sec);
		//std::cout << "complete years since 1970: " << year_i - 1970 << std::endl;	
		month_i += 12 * (year_i -1970) -1 ; //-1, weil man nur beendete Monate zählen möchte.
		//std::cout << "complete months since 1970: " << month_i << std::endl;	
		while(month_i > 0)
		{
			if(month_i % 12 == 0 || month_i % 12 == 1 || month_i % 12 == 3 || month_i % 12 == 5 || month_i % 12 == 7 || month_i % 12 == 8 || month_i % 12 == 10)
			{
				day_i += 31;
				--month_i;
				continue;
			}
			if(month_i % 12 == 4 || month_i % 12 == 6 || month_i % 12 == 9 || month_i % 12 == 11)
			{
				day_i += 30;
				--month_i;
				continue;
			}
			// Die Anzahl der Monate von 1 bis 1970 ist 23640
			if(month_i % 12 == 2 && ((rev_mod(month_i + 23640, 12) % 4 == 0 && rev_mod(month_i + 23640, 12) % 100 != 0) || rev_mod(month_i + 23640, 12) % 400 == 0))
			{
				day_i += 29;
				--month_i;
				continue;
			}
			if(month_i % 12 == 2 && (rev_mod(month_i + 23640, 12) % 4 != 0 || (rev_mod(month_i + 23640, 12) % 100 == 0 && rev_mod(month_i + 23640, 12) % 400 != 0)))
			{
				day_i += 28;
				--month_i;
				continue;
			}
		}
		day_i -= 1; // nur beendete Tage
		//std::cout << "complete days since 1970: " << day_i << std::endl;
		hour_i += 24 * day_i;
		min_i += 60 * hour_i;
		sec_i += 60 * min_i;
		//std::cout << "seconds since 1970: " << sec_i << std::endl;
		return sec_i;
	}
	else
	{
		std::cout << "Ill formed time string." << std::endl;
		return EXIT_FAILURE;
	}
}


Glib::ustring sec_to_ustring(unsigned long int sec_i)
{
	unsigned int sec = sec_i%60;
	unsigned int min = rev_mod(sec_i, 60)%60;
	unsigned int hour = rev_mod(sec_i, 3600)%24;
	unsigned int day = rev_mod(sec_i, 86400) + 1;// angefangene Tage werden gezaehlt
	unsigned int month = 1; // angefangene Monate werden gezaehlt
	while(true) //abbruch durch breaks ...
	{
		if(month % 12 == 0 || month % 12 == 1 || month % 12 == 3 || month % 12 == 5 || month % 12 == 7 || month % 12 == 8 || month % 12 == 10)
		{
			if(day <= 31)
			{break;}
			day -= 31;
			++month;
			continue;
		}
		if(month % 12 == 4 || month % 12 == 6 || month % 12 == 9 || month % 12 == 11)
		{
			if(day <= 30)
			{break;}
			day -= 30;
			++month;
			continue;
		}
		if(month % 12 == 2 && ((rev_mod(month + 23640, 12) % 4 == 0 && rev_mod(month + 23640, 12) % 100 != 0) || rev_mod(month + 23640, 12) % 400 == 0))
		{
			if(day <= 29)
			{break;}
			day -= 29;
			++month;
			continue;
		}
		if(month % 12 == 2 && (rev_mod(month + 23640, 12) % 4 != 0 || (rev_mod(month + 23640, 12) % 100 == 0 && rev_mod(month + 23640, 12) % 400 != 0)))
		{
			if(day <= 28)
			{break;}
			day -= 28;
			++month;
			continue;
		}
	}
	unsigned int year = rev_mod(month, 12) + 1970;
	month = month%12;
	Glib::ustring time_string(int_to_ustring(year) + "-" + int_to_ustring(month) + "-" + int_to_ustring(day) + "T" + int_to_ustring(hour) + ":" + int_to_ustring(min) + ":" +  int_to_ustring(sec) + "Z");
	return time_string;
}


