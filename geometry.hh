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
  const double average_radius_earth = (2*6378.137 + 6356.752)/3.0 * 1000; // 6371.009 km [m]
  // angle = arccos ( sin phi1 * sin phi 2 + cos phi 1 * cos phi 2 * cos (lambda 1 - lambda 2) )
  const double angle = acos( sin(phi1)*sin(phi2) + cos(phi1)*cos(phi2)*cos(theta1-theta2) ); // [rad]
  return average_radius_earth * angle; // [m]
}

double central_angle_acos(const double phi1, const double theta1, const double phi2, const double theta2){
  // angle = arccos ( sin phi1 * sin phi 2 + cos phi 1 * cos phi 2 * cos (lambda 1 - lambda 2) )
  return acos( sin(phi1)*sin(phi2) + cos(phi1)*cos(phi2)*cos(theta1-theta2) ); // [rad]
}

double distance_atan(const double phi1, const double theta1, const double phi2, const double theta2) {
  // angle = (2*a + b)/3
  const double average_radius_earth = (2*6378.137 + 6356.752)/3.0 * 1000; // 6371.009 km [m]
  const double latDiff_half = (phi1 - phi2)/2.0;
  const double longDiff_half = (theta1 - theta2)/2.0;
  const double a = sin(latDiff_half) * sin(latDiff_half) + sin(longDiff_half) * sin(longDiff_half) * cos(phi2) * cos(phi1);
  const double angle = 2 * atan2(sqrt(a), sqrt(1 - a));
  return average_radius_earth * angle; // [m]
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

double horizontal_direction(const double ref_lat, const double ref_lon, const double lat, const double lon){
  const double north_lat(90), north_lon(0);
  const double a = central_angle_atan(lat, lon,             ref_lat, ref_lon);
  const double b = central_angle_atan(north_lat, north_lon, lat, lon);
  const double c = central_angle_atan(ref_lat, ref_lon,     north_lat, north_lon);
  // Use the Spherical law of cosines to get at the angle between a and b
  const double numerator = cos(b) - cos(a) * cos(c);
  const double denominator = sin(a) * sin(c);
  const double angle = acos(numerator / denominator); // [rad] // <90
  if(lat>ref_lat && lon>ref_lon) return M_PI/2-angle; // looking North East
  else if(lat>ref_lat && lon<ref_lon) return M_PI/2+angle; // looking North West
  else if(lat<ref_lat && lon<ref_lon) return 3*M_PI/2-angle; // looking South West
  else /*(lat<ref_lat && lon>ref_lon)*/ return 3*M_PI/2+angle; // looking South East
}

// vertical angle, using distance and elevation difference
// positive is up, negative is down
double angle_v(const double el_ref /* [m] */, const double el /* [m] */, const double dist /* [m] */){
  const int up = el-el_ref>0 ? 1 : -1;
  const double diff_el = abs(el - el_ref); // [m]
  const double angle = atan(diff_el/dist); // [rad]
  return up * angle; // [rad]
}

double angle_v_scaled(const double el_ref, const double el, const double dist){
  double angle = angle_v(el_ref, el, dist); // [rad] 
  if(angle<0) angle *= 0.7;
  return angle; // [rad]
}

// check if two line segments intersect
inline int intersect( const double e1x1, const double e1y1, const double e1x2, const double e1y2,
                      const double e2x1, const double e2y1, const double e2x2, const double e2y2 ){
  const double a1 = (e1y1 - e1y2)/(e1x1 - e1x2);
  const double b1 = e1y1 - a1 * e1x1;
  if ((e2y1 > a1*e2x1+b1 && e2y2 > a1*e2x2+b1) || (e2y1 < a1*e2x1+b1 && e2y2 < a1*e2x2+b1)) return 0; // both points of the second edge lie on the sam    e side of the first edge
  const double a2 = (e2y1 - e2y2)/(e2x1 - e2x2);
  const double b2 = e2y1 - a2 * e2x1;
  if ((e1y1 > a2*e1x1+b2 && e1y2 > a2*e1x2+b2) || (e1y1 < a2*e1x1+b2 && e1y2 < a2*e1x2+b2)) return 0; // both points of the first edge lie on the same     side of the second edge
  return 1;
}

// draw a line from a reference point which is known to be outside to the point in question
// count how many sides intersect with this line
inline bool point_in_triangle_1(double px, double py,
                                double refx, double refy,
                                double x1, double y1, double x2, double y2, double x3, double y3){
  int num_intersections=0;
  num_intersections += intersect(refx,refy, px,py, x1,y1,x2,y2);
  num_intersections += intersect(refx,refy, px,py, x2,y2,x3,y3);
  num_intersections += intersect(refx,refy, px,py, x3,y3,x1,y1);
  return num_intersections==1;
}

inline double signed_area( double x1, double y1, double x2, double y2, double x3, double y3 ){
  return (x1 - x3) * (y2 - y3) - (x2 - x3) * (y1 - y3);
}

inline bool point_in_triangle_2(double px, double py,
                                double x1, double y1, double x2, double y2, double x3, double y3){
  bool b1, b2, b3;
  b1 = signed_area(px, py, x1, y1, x2, y2) < 0;
  b2 = signed_area(px, py, x2, y2, x3, y3) < 0;
  b3 = signed_area(px, py, x3, y3, x1, y1) < 0;
  // and we ignore the case of points lying *on* the boundary
  return ((b1 == b2) && (b2 == b3));
}

#endif

