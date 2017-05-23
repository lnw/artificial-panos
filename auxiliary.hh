
#ifndef AUXILIARY_HH
#define AUXILIARY_HH

#include <cstdlib>
#include <iomanip> //required for setfill()
#include <iostream>
#include <math.h>
#include <sstream>
#include <vector>

#include <libxml++/libxml++.h> //definiert GLib::ustring (das aus dem xml faellt)

using namespace std;

template <typename S, typename T> ostream& operator<<(ostream& s, const pair<S,T>& p) {
  s << "{" << p.first << "," << p.second << "}";
  return s;
}

#define container_output(container) \
  template <typename T> ostream& operator<<(ostream& s, const container<T>& v) \
  { \
  s << "{"; \
  for(typename container<T>::const_iterator x(v.begin());x!=v.end();){ \
    s << *x; \
    if(++x!=v.end()) s << ","; \
  } \
  s << "}"; \
  return s; \
}

container_output(vector);


template <typename T>
std::string to_string_with_precision(const T val, const int p=4) {
  std::stringstream ss;
  ss << std::setprecision(p) << val;
  return ss.str();
}

template <typename T>
Glib::ustring to_ustring_with_precision(const T val, const int p=4) {
  std::stringstream ss;
  ss.precision(p);
  Glib::ustring result;
  ss << val;
  ss >> result;
  return result;
}

template <typename T>
std::string to_string_fixedwidth(const T val, const int n=3) {
  std::stringstream ss;
  ss << std::setw(n) << std::setfill('0') << val;
  return ss.str();
}

template <typename T>
Glib::ustring to_ustring_fixedwidth(const T val, const int n=3) {
  std::stringstream ss;
  Glib::ustring result;
  ss << std::setw(n) << std::setfill('0') << val;
  ss >> result;
  return result;
}

template <typename T>
double to_double(const T& s) {
  std::stringstream ss;
  double result;
  ss << s;
  ss >> result;
  return result;
}

template <typename T>
int to_int(const T& s) {
  std::stringstream ss;
  int result;
  ss << s.raw();
  ss >> result;
  return result;
}


#endif // auxiliary
