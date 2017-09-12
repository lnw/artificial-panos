#ifndef GEOMETRY_HH
#define GEOMETRY_HH

#include <math.h>
#include <vector>

#include "auxiliary.hh"

const double deg2rad = M_PI/180;
const double rad2deg = 180/M_PI;
// angle = (2*a + b)/3
const double average_radius_earth = (2*6378.137 + 6356.752)/3.0 * 1000; // 6371.009 km [m]

// distance between two points on a sphere, without elevation
// phi is the latitude, theta the longitude
// Vincenty's formulae might be better (to take into account earth's oblation)
double distance_acos(const double latA, const double lonA, const double latB, const double lonB);

double central_angle_acos(const double latA, const double lonA, const double latB, const double lonB);

double distance_atan(const double latA, const double lonA, const double latB, const double lonB);

double central_angle_atan(const double latA, const double lonA, const double latB, const double lonB);

// horizontal angle, ie, angle between two great circles
double angle_h(const double latA, const double lonA,
               const double latB, const double lonB,
               const double latC, const double lonC);

double horizontal_direction(const double ref_lat, const double ref_lon,
                            const double lat, const double lon);

// bearing, starting from ref
// where N: 0, E:90, S:+/-180, W:-90
double bearing(const double ref_lat, const double ref_lon,
               const double lat, const double lon);

// destination when going from (lat/lon) a distance dist with bearing b
// bearing from -pi .. pi, 0 is north
pair<double,double> destination(const double ref_lat, const double ref_lon, const double dist, const double b);

// vertical angle, using distance and elevation difference
// positive is up, negative is down
double angle_v(const double el_ref /* [m] */, const double el /* [m] */, const double dist /* [m] */);

double angle_v_scaled(const double el_ref, const double el, const double dist);

// check if two line segments intersect
int intersect( const double e1x1, const double e1y1, const double e1x2, const double e1y2,
                      const double e2x1, const double e2y1, const double e2x2, const double e2y2 );

// draw a line from a reference point which is known to be outside to the point in question
// count how many sides intersect with this line
bool point_in_triangle_1(const double px, const double py,
                                const double refx, const double refy,
                                const double x1, const double y1, const double x2, const double y2, const double x3, const double y3);

// area of triangle, but more importantly, are the points CW or CCW?
double signed_area( double x1, double y1, double x2, double y2, double x3, double y3 );

// is point (px/py) inside a triangle?
bool point_in_triangle_2(double px, double py,
                                double x1, double y1, double x2, double y2, double x3, double y3);

#endif

