#ifndef __CONVERSIONS_H__
#define __CONVERSIONS_H__

#ifndef PI
#define PI 3.141592654
#endif

#include <libxml++/libxml++.h> // koennte fuer die deklaration notwendig sein

//double ustring_to_double(Glib::ustring ustring);
//double string_to_double(std::string string);
template <typename T> double to_double(const T& s){
	std::stringstream ss;
	double result;
	ss << s;
	ss >> result;
	return result;
}

//int ustring_to_int(Glib::ustring ustring);
template <typename T> int to_int(const T& s){
	std::stringstream ss;
	int result;
	ss << s.raw();
	ss >> result;
	return result;
}

Glib::ustring double_to_ustring(double d);
Glib::ustring int_to_ustring(int i);

//reverse modulus:  gibt an wieoft a ganz in b passt
int rev_mod(int i, int j);

unsigned long int ustring_to_sec(Glib::ustring time);
Glib::ustring sec_to_ustring(unsigned long int sec_i);

#endif //__CONVERSIONS_H__
