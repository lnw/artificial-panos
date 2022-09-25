#pragma once

#include <cmath>

#include "auxiliary.hh"

constexpr double deg2rad = M_PI / 180;
constexpr double rad2deg = 180 / M_PI;
// angle = (2*a + b)/3
constexpr double average_radius_earth = (2 * 6378.137 + 6356.752) / 3.0 * 1000; // 6371.009 km [m]

// distance between two points on a sphere, without elevation
// phi is the latitude, theta the longitude
// Vincenty's formulae might be better (to take into account earth's oblation)
// double distance_acos(const double latA, const double lonA, const double latB, const double lonB);

template <typename T>
inline T distance_atan(const T latA, const T lonA, const T latB, const T lonB) {
  const T latDiff_half = (latA - latB) / 2.0;
  const T longDiff_half = (lonA - lonB) / 2.0;
  const T a = std::sin(latDiff_half) * std::sin(latDiff_half) + std::sin(longDiff_half) * std::sin(longDiff_half) * std::cos(latB) * std::cos(latA);
  const T angle = 2 * std::atan2(std::sqrt(a), std::sqrt(1 - a));
  return average_radius_earth * angle; // [m]
}

double central_angle_acos(const double latA, const double lonA, const double latB, const double lonB);

double central_angle_atan(const double latA, const double lonA, const double latB, const double lonB);

// horizontal angle, ie, angle between two great circles
double angle_h(const double latA, const double lonA,
               const double latB, const double lonB,
               const double latC, const double lonC);

double horizontal_direction(const double ref_lat, const double ref_lon,
                            const double lat, const double lon);

// bearing, starting from ref
// where N: 0, E:90, S:+/-180, W:-90
// input and output in rad
double bearing(double ref_lat, double ref_lon, double lat, double lon);

// destination when going from (lat/lon) a distance dist with bearing b
// bearing from -pi .. pi, 0 is north
std::pair<double, double> destination(double ref_lat, double ref_lon, double dist, double b);

// vertical angle, using distance and elevation difference
// positive is up, negative is down
template <typename T>
T angle_v(T el_ref /* [m] */, T el /* [m] */, T dist /* [m] */);

template <typename T>
T angle_v_scaled(T el_ref, T el, T dist);

// check if two line segments intersect
template <typename T>
constexpr int intersect(const T e1x1, const T e1y1, const T e1x2, const T e1y2,
                        const T e2x1, const T e2y1, const T e2x2, const T e2y2) {
  const auto a1 = (e1y1 - e1y2) / (e1x1 - e1x2);
  const auto b1 = e1y1 - a1 * e1x1;
  if ((e2y1 > a1 * e2x1 + b1 && e2y2 > a1 * e2x2 + b1) || (e2y1 < a1 * e2x1 + b1 && e2y2 < a1 * e2x2 + b1))
    return 0; // both points of the second edge lie on the same side of the first edge
  const auto a2 = (e2y1 - e2y2) / (e2x1 - e2x2);
  const auto b2 = e2y1 - a2 * e2x1;
  if ((e1y1 > a2 * e1x1 + b2 && e1y2 > a2 * e1x2 + b2) || (e1y1 < a2 * e1x1 + b2 && e1y2 < a2 * e1x2 + b2))
    return 0; // both points of the first edge lie on the same side of the second edge
  return 1;
}


// draw a line from a reference point which is known to be outside to the point in question
// count how many sides intersect with this line
template <typename T>
constexpr bool point_in_triangle_1(const T px, const T py, const T refx, const T refy,
                                   const T x1, const T y1, const T x2, const T y2, const T x3, const T y3) {
  const auto num_intersections = intersect(refx, refy, px, py, x1, y1, x2, y2) +
                                 intersect(refx, refy, px, py, x2, y2, x3, y3) +
                                 intersect(refx, refy, px, py, x3, y3, x1, y1);
  return (num_intersections == 1);
}


// twice the area of triangle, but more importantly, are the points CW or CCW?
// cross product of two edges of the triangle, in 3D, z component of result is twice the area
template <typename T>
constexpr T signed_area(T x1, T y1, T x2, T y2, T x3, T y3) {
  return (x1 - x3) * (y2 - y3) - (x2 - x3) * (y1 - y3);
}


// is point (px/py) inside a triangle?
// if the point is inside the triangle, all areas have the same sign, otherwise one has opposite sign
template <typename T>
constexpr bool point_in_triangle_2(T px, T py, T x1, T y1, T x2, T y2, T x3, T y3) {
  const bool b1 = (signed_area(px, py, x1, y1, x2, y2) < 0);
  const bool b2 = (signed_area(px, py, x2, y2, x3, y3) < 0);
  const bool b3 = (signed_area(px, py, x3, y3, x1, y1) < 0);
  // and we ignore the case of points lying *on* the boundary
  return ((b1 == b2) && (b2 == b3));
}
