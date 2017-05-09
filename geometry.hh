#ifndef GEOMETRY_HH
#define GEOMETRY_HH

#include <math.h>
#include <vector>

#include "auxiliary.hh"

const double deg2rad_const = M_PI/180;
const double rad2deg_const = 180/M_PI;

template <typename T> class coord {
  public:
  T lat, lon;

  coord(T lat, T lon): lat(lat), lon(lon) {};

  void deg2rad(){ 
    lat *= deg2rad_const;
    lon *= deg2rad_const;
  }
  void rad2deg(){ 
    lat *= rad2deg_const;
    lon *= rad2deg_const;
  }
};

// distance between two points on a sphere, without elevation
// phi is the latitude, theta the longitude
// Vincenty's formulae might be better (to take into account earth's oblation)
double distance_acos(const double phi1, const double theta1, const double phi2, const double theta2){
  // angle = (2*a + b)/3
  const double average_radius_earth = (2*6378.137 + 6356.752)/3.0; // 6371.009 km
  // angle = arccos ( sin phi1 * sin phi 2 + cos phi 1 * cos phi 2 * cos (lambda 1 - lambda 2) )
  const double angle = acos( sin(phi1)*sin(phi2) + cos(phi1)*cos(phi2)*cos(theta1-theta2) ); // [rad]
  return average_radius_earth * angle; // [km]
}

double central_angle_acos(const double phi1, const double theta1, const double phi2, const double theta2){
  // angle = arccos ( sin phi1 * sin phi 2 + cos phi 1 * cos phi 2 * cos (lambda 1 - lambda 2) )
  return acos( sin(phi1)*sin(phi2) + cos(phi1)*cos(phi2)*cos(theta1-theta2) ); // [rad]
}

double distance_atan(const double phi1, const double theta1, const double phi2, const double theta2) {
  // angle = (2*a + b)/3
  const double average_radius_earth = (2*6378.137 + 6356.752)/3.0; // 6371.009 km
  const double latDiff_half = (phi1 - phi2)/2.0;
  const double longDiff_half = (theta1 - theta2)/2.0;
  const double a = sin(latDiff_half) * sin(latDiff_half) + sin(longDiff_half) * sin(longDiff_half) * cos(phi2) * cos(phi1);
  const double angle = 2 * atan2(sqrt(a), sqrt(1 - a));
  return average_radius_earth * angle; // [km]
}

double central_angle_atan(const double phi1, const double theta1, const double phi2, const double theta2) {
  const double latDiff_half = (phi1 - phi2)/2.0;
  const double longDiff_half = (theta1 - theta2)/2.0;
  const double a = sin(latDiff_half) * sin(latDiff_half) + sin(longDiff_half) * sin(longDiff_half) * cos(phi2) * cos(phi1);
  return 2 * atan2(sqrt(a), sqrt(1 - a)); // [rad]
}

// horizontal angle, ie, angle between two great circles
double angle_h( const double phi_A, const double theta_A, const double phi_B, const double theta_B, const double phi_C, const double theta_C) {
  // Convert real distances to unit sphere distances
  const double a = central_angle_atan(phi_B, theta_B, phi_C, theta_C);
  const double b = central_angle_atan(phi_A, theta_A, phi_C, theta_C);
  const double c = central_angle_atan(phi_A, theta_A, phi_B, theta_B);
  // Use the Spherical law of cosines to get at the angle between a and b
  const double numerator = cos(b) - cos(a) * cos(c);
  const double denominator = sin(a) * sin(c);
  return acos(numerator / denominator); // [rad]
}

// vertical angle, using distance and elevation difference
// in rad
// positive is up, negative is down
double angle_v(const double phi1, const double theta1, const double z1, const double phi2, const double theta2, const double z2){
  const double dist_h = distance_atan(phi1, theta1, phi2, theta2); // km
  const int up = z1-z2>0 ? 1 : -1;
  const double diff_z = abs(z1 - z2); // m
  const double angle = atan(diff_z/(1000*dist_h)); // rad
  return up * angle; // [rad]
}

double angle_v_scaled(const double phi1, const double theta1, const double z1, const double phi2, const double theta2, const double z2){
  double angle = angle_v(phi1, theta1, z1, phi2, theta2, z2);
  if(angle<0) angle *= 0.7;
  return angle; // [rad]
}

#endif

