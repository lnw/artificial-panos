// test for internal2cart

#define CATCH_CONFIG_MAIN
#include "/home/lukas/bin/Catch/single_include/catch.hpp"

#include <assert.h>
#include <cmath>
#include <fstream>
#include <vector>

#include "auxiliary.hh"
#include "geometry.hh"

using namespace std;

TEST_CASE( "gcd", "gcd" ) {
  const double deg2rad = M_PI/180.0;
  const double rad2deg = 180.0/M_PI;
  double p1(49*deg2rad), p2(50*deg2rad), t1(6*deg2rad);
  CHECK( distance_atan(p1,t1,p2,t1) == Approx(111.195) );
  CHECK( distance_acos(p1,t1,p2,t1) == Approx(111.195) );
  double p3(89*deg2rad), p4(90*deg2rad), t2(6*deg2rad);
  CHECK( distance_atan(p3,t2,p4,t2) == Approx(111.195) );
  CHECK( distance_acos(p3,t2,p4,t2) == Approx(111.195) );
  double p5(89*deg2rad), p6(90*deg2rad), t3(0*deg2rad), t4(20*deg2rad);
  CHECK( distance_atan(p5,t3,p6,t3) == Approx(111.195) );
  CHECK( distance_acos(p5,t3,p6,t3) == Approx(111.195) );
  CHECK( distance_atan(p5,t3,p6,t4) == Approx(111.195) );
  CHECK( distance_acos(p5,t3,p6,t4) == Approx(111.195) );
}

