
#include <math.h>
#include <vector>

#include "auxiliary.hh"
#include "geometry.hh"

// distance between two points on a sphere, without elevation
// phi is the latitude, theta the longitude
// Vincenty's formulae might be better (to take into account earth's oblation)
double distance_acos(const double latA, const double lonA, const double latB, const double lonB){
  // angle = arccos ( sin latA * sin phi 2 + cos phi 1 * cos phi 2 * cos (lambda 1 - lambda 2) )
  const double angle = acos( sin(latA)*sin(latB) + cos(latA)*cos(latB)*cos(lonA-lonB) ); // [rad]
  return average_radius_earth * angle; // [m]
}

double central_angle_acos(const double latA, const double lonA, const double latB, const double lonB){
  // angle = arccos ( sin latA * sin phi 2 + cos phi 1 * cos phi 2 * cos (lambda 1 - lambda 2) )
  return acos( sin(latA)*sin(latB) + cos(latA)*cos(latB)*cos(lonA-lonB) ); // [rad]
}

// distance between A and B, using the atan
// input in radians
double distance_atan(const double latA, const double lonA, const double latB, const double lonB) {
  const double latDiff_half = (latA - latB)/2.0;
  const double longDiff_half = (lonA - lonB)/2.0;
  const double a = sin(latDiff_half) * sin(latDiff_half) + sin(longDiff_half) * sin(longDiff_half) * cos(latB) * cos(latA);
  const double angle = 2 * atan2(sqrt(a), sqrt(1 - a));
  return average_radius_earth * angle; // [m]
}

double central_angle_atan(const double latA, const double lonA, const double latB, const double lonB) {
  const double latDiff_half = (latA - latB)/2.0;
  const double longDiff_half = (lonA - lonB)/2.0;
  const double a = sin(latDiff_half) * sin(latDiff_half) + sin(longDiff_half) * sin(longDiff_half) * cos(latB) * cos(latA);
  return 2 * atan2(sqrt(a), sqrt(1 - a)); // [rad]
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
                            const double lat, const double lon){
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

// bearing, starting from ref
// where N: 0, E:90, S:+/-180, W:-90
double bearing(const double ref_lat, const double ref_lon,
               const double lat, const double lon){
  const double Dlon = lon - ref_lon;
  const double x = cos(lat)*sin(Dlon);
  const double y = cos(ref_lat)*sin(lat) - sin(ref_lat)*cos(lat)*cos(Dlon);
  return atan2(x,y);
}

// destination when going from (lat/lon) a distance dist with bearing b
// bearing from -pi .. pi, 0 is north
pair<double,double> destination(const double ref_lat, const double ref_lon, const double dist, const double b){
  const double c_angle = dist/average_radius_earth; // central angle
  const double lat = asin( sin(ref_lat)*cos(c_angle) + cos(ref_lat)*sin(c_angle)*cos(b) );
  const double lon = ref_lon + atan2(sin(b)*sin(c_angle)*cos(ref_lat),
                                     cos(c_angle)-sin(ref_lat)*sin(lat));
  return make_pair(lat,lon);
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
int intersect( const double e1x1, const double e1y1, const double e1x2, const double e1y2,
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
bool point_in_triangle_1(const double px, const double py,
                         const double refx, const double refy,
                         const double x1, const double y1, const double x2, const double y2, const double x3, const double y3){
  const int num_intersections = intersect(refx,refy, px,py, x1,y1,x2,y2)
                              + intersect(refx,refy, px,py, x2,y2,x3,y3)
                              + intersect(refx,refy, px,py, x3,y3,x1,y1);
  return num_intersections==1;
}

// twice the area of triangle, but more importantly, are the points CW or CCW?
// cross product of two edges of the triangle, in 3D, z component of result is twice the area
double signed_area( double x1, double y1, double x2, double y2, double x3, double y3 ){
  return (x1 - x3) * (y2 - y3) - (x2 - x3) * (y1 - y3);
}

// is point (px/py) inside a triangle?
// if the point is inside the triangle, all areas have the same sign, otherwise one has opposite sign
bool point_in_triangle_2(double px, double py,
                         double x1, double y1, double x2, double y2, double x3, double y3){
  bool b1 = signed_area(px, py, x1, y1, x2, y2) < 0;
  bool b2 = signed_area(px, py, x2, y2, x3, y3) < 0;
  bool b3 = signed_area(px, py, x3, y3, x1, y1) < 0;
  // and we ignore the case of points lying *on* the boundary
  return ((b1 == b2) && (b2 == b3));
}

