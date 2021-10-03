#include <cmath>
#include <vector>

#include "auxiliary.hh"
#include "geometry.hh"

// distance between two points on a sphere, without elevation
// phi is the latitude, theta the longitude
// Vincenty's formulae might be better (to take into account earth's oblation)
double distance_acos(const double latA, const double lonA, const double latB, const double lonB) {
  // angle = arccos ( sin latA * sin phi 2 + cos phi 1 * cos phi 2 * cos (lambda 1 - lambda 2) )
  const double angle = std::acos(std::sin(latA) * std::sin(latB) + std::cos(latA) * std::cos(latB) * std::cos(lonA - lonB)); // [rad]
  return average_radius_earth * angle;                                                                                       // [m]
}

double central_angle_acos(const double latA, const double lonA, const double latB, const double lonB) {
  // angle = arccos ( sin latA * sin phi 2 + cos phi 1 * cos phi 2 * cos (lambda 1 - lambda 2) )
  return acos(sin(latA) * sin(latB) + cos(latA) * cos(latB) * cos(lonA - lonB)); // [rad]
}

// // distance between A and B, using the atan2
// // input in radians
// double distance_atan(const double latA, const double lonA, const double latB, const double lonB) {
//   const double latDiff_half = (latA - latB) / 2.0;
//   const double longDiff_half = (lonA - lonB) / 2.0;
//   const double a = sin(latDiff_half) * sin(latDiff_half) + sin(longDiff_half) * sin(longDiff_half) * cos(latB) * cos(latA);
//   const double angle = 2 * atan2(sqrt(a), sqrt(1 - a));
//   return average_radius_earth * angle; // [m]
// }

// the angle between A-centre-B, using the atan2
// input in radians
double central_angle_atan(const double latA, const double lonA, const double latB, const double lonB) {
  const double latDiff_half = (latA - latB) / 2.0;
  const double longDiff_half = (lonA - lonB) / 2.0;
  const double a = sin(latDiff_half) * sin(latDiff_half) + sin(longDiff_half) * sin(longDiff_half) * cos(latB) * cos(latA);
  return 2 * std::atan2(std::sqrt(a), std::sqrt(1 - a)); // [rad]
}

// horizontal angle, ie, angle between two great circles
double angle_h(const double latA, const double lonA,
               const double latB, const double lonB,
               const double latC, const double lonC) {
  // Convert real distances to unit sphere distances
  const double a = central_angle_atan(latB, lonB, latC, lonC);
  const double b = central_angle_atan(latA, lonA, latC, lonC);
  const double c = central_angle_atan(latA, lonA, latB, lonB);
  // Use the Spherical law of cosines to get at the angle between a and b
  const double numerator = cos(b) - cos(a) * cos(c);
  const double denominator = sin(a) * sin(c);
  return acos(numerator / denominator); // [rad]
}

double horizontal_direction(const double ref_lat, const double ref_lon,
                            const double lat, const double lon) {
  const double north_lat(90), north_lon(0);
  const double a = central_angle_atan(lat, lon, ref_lat, ref_lon);
  const double b = central_angle_atan(north_lat, north_lon, lat, lon);
  const double c = central_angle_atan(ref_lat, ref_lon, north_lat, north_lon);
  // Use the Spherical law of cosines to get at the angle between a and b
  const double numerator = std::cos(b) - std::cos(a) * std::cos(c);
  const double denominator = std::sin(a) * std::sin(c);
  const double angle = std::acos(numerator / denominator); // [rad] // <90
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
double bearing(const double ref_lat, const double ref_lon,
               const double lat, const double lon) {
  const double Dlon = lon - ref_lon;
  const double x = std::cos(lat) * std::sin(Dlon);
  const double y = std::cos(ref_lat) * std::sin(lat) - std::sin(ref_lat) * std::cos(lat) * std::cos(Dlon);
  return atan2(x, y);
}

// destination when going from (lat/lon) a distance dist with bearing b
// bearing from -pi .. pi, 0 is north
std::pair<double, double> destination(const double ref_lat, const double ref_lon, const double dist, const double b) {
  const double c_angle = dist / average_radius_earth; // central angle
  const double lat = std::asin(std::sin(ref_lat) * std::cos(c_angle) + std::cos(ref_lat) * std::sin(c_angle) * std::cos(b));
  const double lon = ref_lon + std::atan2(std::sin(b) * std::sin(c_angle) * std::cos(ref_lat),
                                          std::cos(c_angle) - std::sin(ref_lat) * std::sin(lat));
  return std::make_pair(lat, lon);
}

// vertical angle, using distance and elevation difference
// positive is up, negative is down
template <typename T>
T angle_v(const T el_ref /* [m] */, const T el /* [m] */, const T dist /* [m] */) {
  const int up = el - el_ref > 0 ? 1 : -1;
  const double diff_el = std::abs(el - el_ref);   // [m]
  const double angle = std::atan(diff_el / dist); // [rad]
  return up * angle;                              // [rad]
}
template double angle_v(const double el_ref, const double el, const double dist);
template float angle_v(const float el_ref, const float el, const float dist);

template <typename T>
T angle_v_scaled(const T el_ref, const T el, const T dist) {
  auto angle = angle_v(el_ref, el, dist); // [rad]
  if (angle < 0)
    angle *= 0.7;
  return angle; // [rad]
}
