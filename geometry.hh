#pragma once

#include <cmath>
#include <numbers>

#include "auxiliary.hh"
#include "degrad.hh"
#include "latlon.hh"

// angle = (2*a + b)/3
constexpr double average_radius_earth = (2 * 6378.137 + 6356.752) / 3.0 * 1000; // 6371.009 km [m]

// distance between two points on a sphere, without elevation
// phi is the latitude, theta the longitude
// Vincenty's formulae might be better (to take into account earth's oblation)

template <typename T>
constexpr T distance_acos(const LatLon<T, Unit::rad> A, const LatLon<T, Unit::rad> B) {
  const T angle = central_angle_acos(A, B);
  return average_radius_earth * angle; // [m]
}

template <typename T>
constexpr T distance_atan(const LatLon<T, Unit::rad> A, const LatLon<T, Unit::rad> B) {
  const T angle = central_angle_atan(A, B);
  return average_radius_earth * angle; // [m]
}

template <typename T>
constexpr T central_angle_acos(LatLon<T, Unit::rad> A, LatLon<T, Unit::rad> B) {
  const auto [latA, lonA] = A;
  const auto [latB, lonB] = B;
  return std::acos(std::sin(latA) * std::sin(latB) + std::cos(latA) * std::cos(latB) * std::cos(lonA - lonB)); // [rad]
}

template <typename T>
constexpr T central_angle_atan(LatLon<T, Unit::rad> A, LatLon<T, Unit::rad> B) {
  const auto [latA, lonA] = A;
  const auto [latB, lonB] = B;
  const T latDiff_half = (latA - latB) / 2.0;
  const T longDiff_half = (lonA - lonB) / 2.0;
  const T a = std::sin(latDiff_half) * std::sin(latDiff_half) + std::sin(longDiff_half) * std::sin(longDiff_half) * std::cos(latB) * std::cos(latA);
  return 2 * std::atan2(std::sqrt(a), std::sqrt(1 - a)); // [rad]
}

// horizontal angle at B, ie, angle between two great circles
template <typename T>
constexpr T angle_h(LatLon<T, Unit::rad> A, LatLon<T, Unit::rad> B, LatLon<T, Unit::rad> C) {
  // Convert real distances to unit sphere distances
  const T a = central_angle_atan(B, C);
  const T b = central_angle_atan(A, C);
  const T c = central_angle_atan(A, B);
  // Use the Spherical law of cosines to get at the angle between a and b
  const T numerator = std::cos(b) - std::cos(a) * std::cos(c);
  const T denominator = std::sin(a) * std::sin(c);
  return std::acos(numerator / denominator); // [rad]
}

// horizontal angle north--ref--dest
template <typename T>
constexpr T horizontal_direction(LatLon<T, Unit::rad> ref, LatLon<T, Unit::rad> dest) {
  const auto [ref_lat, ref_lon] = ref;
  const auto [lat, lon] = dest;
  const LatLon<T, Unit::rad> north(M_PI / 2, 0);
  const T angle = angle_h(north, ref, dest);
  if (lat > ref_lat && lon > ref_lon)
    return M_PI / 2 - angle; // looking North East
  if (lat > ref_lat && lon < ref_lon)
    return M_PI / 2 + angle; // looking North West
  if (lat < ref_lat && lon < ref_lon)
    return 3 * M_PI / 2 - angle; // looking South West
  // (lat<ref_lat && lon>ref_lon)
  return 3 * M_PI / 2 + angle; // looking South East
}

// bearing, starting from ref
// where N: 0, E:90, S:+/-180, W:-90
// input and output in rad
template <typename T>
constexpr T bearing(const LatLon<T, Unit::rad> ref, const LatLon<T, Unit::rad> dest) {
  const auto [ref_lat, ref_lon] = ref;
  const auto [dest_lat, dest_lon] = dest;
  const T diff_lon = dest_lon - ref_lon;
  const T x = std::cos(dest_lat) * std::sin(diff_lon);
  const T y = std::cos(ref_lat) * std::sin(dest_lat) - std::sin(ref_lat) * std::cos(dest_lat) * std::cos(diff_lon);
  return std::atan2(x, y);
}


// destination point when going from (lat/lon) a distance dist with bearing b
// bearing from -pi .. pi, 0 is north
template <typename T>
constexpr LatLon<T, Unit::rad> destination(LatLon<T, Unit::rad> point_ref, const T dist, const T b) {
  const auto [ref_lat, ref_lon] = point_ref;
  const T central_angle = dist / average_radius_earth; // central angle
  const T lat = std::asin(std::sin(ref_lat) * std::cos(central_angle) + std::cos(ref_lat) * std::sin(central_angle) * std::cos(b));
  const T lon = ref_lon + std::atan2(std::sin(b) * std::sin(central_angle) * std::cos(ref_lat),
                                     std::cos(central_angle) - std::sin(ref_lat) * std::sin(lat));
  return {lat, lon};
}


// vertical angle between two points, using distance and elevation difference
// positive is up, negative is down
template <typename T>
T angle_v(const T elevation_ref /* [m] */, const T elevation /* [m] */, const T dist /* [m] */) {
  const int up = elevation - elevation_ref > 0 ? 1 : -1;
  const T diff_el = std::abs(elevation - elevation_ref); // [m]
  const T angle = std::atan(diff_el / dist);             // [rad]
  return up * angle;                                     // [rad]
}

template <typename T>
T angle_v_scaled(const T elevation_ref, const T elevation, const T dist, float factor) {
  auto angle = angle_v(elevation_ref, elevation, dist); // [rad]
  if (angle < 0)
    angle *= factor;
  return angle; // [rad]
}

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
