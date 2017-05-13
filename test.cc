
#define CATCH_CONFIG_MAIN
#include "/home/lukas/bin/Catch/single_include/catch.hpp"

#include <assert.h>
#include <fstream>
#include <vector>

#include "auxiliary.hh"
#include "geometry.hh"

using namespace std;


TEST_CASE( "great circle distance", "gcd" ) {
  double p1(49*deg2rad_const), p2(50*deg2rad_const), t1(6*deg2rad_const);
  CHECK( distance_atan(p1,t1,p2,t1) == Approx(111195.0) );
  CHECK( distance_acos(p1,t1,p2,t1) == Approx(111195.0) );
  double p3(89*deg2rad_const), p4(90*deg2rad_const), t2(6*deg2rad_const);
  CHECK( distance_atan(p3,t2,p4,t2) == Approx(111195.0) );
  CHECK( distance_acos(p3,t2,p4,t2) == Approx(111195.0) );
  double p5(89*deg2rad_const), p6(90*deg2rad_const), t3(0*deg2rad_const), t4(20*deg2rad_const);
  CHECK( distance_atan(p5,t3,p6,t3) == Approx(111195.0) );
  CHECK( distance_acos(p5,t3,p6,t3) == Approx(111195.0) );
  CHECK( distance_atan(p5,t3,p6,t4) == Approx(111195.0) );
  CHECK( distance_acos(p5,t3,p6,t4) == Approx(111195.0) );
}

TEST_CASE( "bearing", "bearing" ) {
  double latA(50*deg2rad_const), latB(51*deg2rad_const), lonA(5*deg2rad_const), lonB=5*deg2rad_const;
  CHECK( bearing(latA,lonA,latB,lonB) == Approx(0.0*deg2rad_const) ); //  north
  latA=50*deg2rad_const, latB=50*deg2rad_const, lonA=5*deg2rad_const, lonB=6*deg2rad_const;
  CHECK( bearing(latA,lonA,latB,lonB) == Approx(89.617*deg2rad_const) ); // east
  latA=51*deg2rad_const, latB=50*deg2rad_const, lonA=5*deg2rad_const, lonB=5*deg2rad_const;
  CHECK( bearing(latA,lonA,latB,lonB) == Approx(180.0*deg2rad_const) ); // south
  latA=50*deg2rad_const, latB=50*deg2rad_const, lonA=6*deg2rad_const, lonB=5*deg2rad_const;
  CHECK( bearing(latA,lonA,latB,lonB) == Approx(-89.617*deg2rad_const) ); // west
  // equator
  latA=0*deg2rad_const, latB=0*deg2rad_const, lonA=5*deg2rad_const, lonB=6*deg2rad_const;
  CHECK( bearing(latA,lonA,latB,lonB) == Approx(90*deg2rad_const) ); // east
  latA=0*deg2rad_const, latB=0*deg2rad_const, lonA=6*deg2rad_const, lonB=5*deg2rad_const;
  CHECK( bearing(latA,lonA,latB,lonB) == Approx(-90*deg2rad_const) ); // west
  // cross hemispheres
  latA=0*deg2rad_const, latB=0*deg2rad_const, lonA=-5*deg2rad_const, lonB=5*deg2rad_const;
  CHECK( bearing(latA,lonA,latB,lonB) == Approx(90.0*deg2rad_const) ); // east
  latA=0*deg2rad_const, latB=0*deg2rad_const, lonA=5*deg2rad_const, lonB=-5*deg2rad_const;
  CHECK( bearing(latA,lonA,latB,lonB) == Approx(-90.0*deg2rad_const) ); // west
  latA=0*deg2rad_const, latB=0*deg2rad_const, lonA=170*deg2rad_const, lonB=-170*deg2rad_const;
  CHECK( bearing(latA,lonA,latB,lonB) == Approx(90.0*deg2rad_const) ); // east
  latA=0*deg2rad_const, latB=0*deg2rad_const, lonA=-170*deg2rad_const, lonB=170*deg2rad_const;
  CHECK( bearing(latA,lonA,latB,lonB) == Approx(-90.0*deg2rad_const) ); // west
  // cross poles
  latA=85*deg2rad_const, latB=85*deg2rad_const, lonA=10*deg2rad_const, lonB=-170*deg2rad_const;
  CHECK( bearing(latA,lonA,latB,lonB) == Approx(0.0*deg2rad_const) ); // north
  latA=85*deg2rad_const, latB=85*deg2rad_const, lonA=-170*deg2rad_const, lonB=10*deg2rad_const;
  CHECK( bearing(latA,lonA,latB,lonB) == Approx(0.0*deg2rad_const) ); // still north
  latA=-85*deg2rad_const, latB=-85*deg2rad_const, lonA=10*deg2rad_const, lonB=-170.001*deg2rad_const;
  CHECK( bearing(latA,lonA,latB,lonB) == Approx(180.0*deg2rad_const) ); // south
  latA=-85*deg2rad_const, latB=-85*deg2rad_const, lonA=-170*deg2rad_const, lonB=9.999*deg2rad_const;
  CHECK( bearing(latA,lonA,latB,lonB) == Approx(180.0*deg2rad_const) ); // south
}
