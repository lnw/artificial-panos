
#include <cmath>
#include <vector>

// distance between two points on a sphere, without elevation
// phi is the latitude, theta the longitude
// Vincenty's formulae might be better
double distance(double phi1, double phi2, double theta1, double theta2){
  // angle = (2*a + b)/3
  const double average_radius_earth = (2*6378.137 + 6356.752)/3.0; // 6371.009 km
  // angle = arccos ( sin phi1 * sin phi 2 + cos phi 1 * cos phi 2 * cos (lambda 1 - lambda 2) )
  const double angle = acos( sin(phi1)*sin(phi2) + cos(phi1)*cos(phi2)*cos(theta1-theta2) ); // [radians]
  return average_radius_earth * angle; // [km]
}

// vertical angle

// horizontal angle

