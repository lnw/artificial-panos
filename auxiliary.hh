#ifndef AUXILIARY_HH
#define AUXILIARY_HH

#include <iostream>
#include <vector>
#include <sstream>
#include <iomanip>

using namespace std;

template <typename S, typename T> ostream& operator<<(ostream& s, const pair<S,T>& p)
{
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
std::string to_string_with_precision(const T a_value, const int n = 3)
{
  std::ostringstream out;
  out << std::setprecision(n) << a_value;
  return out.str();
}

template <typename T>
std::string to_string_fixedwidth(const T a_value, const int n = 3)
{
  std::ostringstream out;
  int num_digits = to_string(a_value).length();
  if(num_digits<n){
    for(int i=0; i<n-num_digits; i++){
      out << "0";
    }
  }
  out << to_string(a_value);
  return out.str();
}

#endif
