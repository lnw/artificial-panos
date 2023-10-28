#pragma once

#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <set>
#include <sstream>
#include <vector>


template <typename S, typename T>
std::ostream& operator<<(std::ostream& s, const std::pair<S, T>& p) {
  s << "{" << p.first << "," << p.second << "}";
  return s;
}

#define container_output(container)                                         \
template <typename T>                                                       \
std::ostream& operator<<(std::ostream& s, const container<T>& v) {          \
  s << "{";                                                                 \
  for (typename container<T>::const_iterator x(v.begin()); x != v.end();) { \
    s << *x;                                                                \
    if (++x != v.end())                                                     \
      s << ",";                                                             \
  }                                                                         \
  s << "}";                                                                 \
  return s;                                                                 \
}

container_output(std::vector)

template <typename To, typename From>
auto to_stringish_with_precision(const From val, const int p = 4) {
  std::stringstream ss;
  ss.precision(p);
  To result;
  ss << val;
  ss >> result;
  return result;
}

template <typename To, typename From>
auto to_stringish_fixedwidth(const From val, const int n = 3) {
  std::stringstream ss;
  To result;
  ss << std::setw(n) << std::setfill('0') << val;
  ss >> result;
  return result;
}

template <typename To, typename From>
auto convert_from_stringish(const From& s) {
  std::stringstream ss;
  To result;
  ss << s;
  ss >> result;
  return result;
}

// is t in [b, e[ ?
template <class T1, class T2, class T3>
constexpr bool is_in_range(T1 t, T2 b, T3 e) noexcept {
  return b <= t && t < e;
}
