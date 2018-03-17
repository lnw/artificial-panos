
#define CATCH_CONFIG_MAIN
#include "/home/lukas/bin/Catch/single_include/catch.hpp"

#include <cassert>
#include <fstream>
#include <vector>

#include "auxiliary.hh"
#include "geometry.hh"
#include "colour.hh"

using namespace std;


TEST_CASE( "great circle distance", "gcd" ) {
  double p1(49*deg2rad), p2(50*deg2rad), t1(6*deg2rad);
  CHECK( distance_atan(p1,t1,p2,t1) == Approx(111195.0) );
  CHECK( distance_acos(p1,t1,p2,t1) == Approx(111195.0) );
  double p3(89*deg2rad), p4(90*deg2rad), t2(6*deg2rad);
  CHECK( distance_atan(p3,t2,p4,t2) == Approx(111195.0) );
  CHECK( distance_acos(p3,t2,p4,t2) == Approx(111195.0) );
  double p5(89*deg2rad), p6(90*deg2rad), t3(0*deg2rad), t4(20*deg2rad);
  CHECK( distance_atan(p5,t3,p6,t3) == Approx(111195.0) );
  CHECK( distance_acos(p5,t3,p6,t3) == Approx(111195.0) );
  CHECK( distance_atan(p5,t3,p6,t4) == Approx(111195.0) );
  CHECK( distance_acos(p5,t3,p6,t4) == Approx(111195.0) );
}

TEST_CASE( "bearing", "bearing" ) {
  double latA(50*deg2rad), latB(51*deg2rad), lonA(5*deg2rad), lonB=5*deg2rad;
  CHECK( bearing(latA,lonA,latB,lonB) == Approx(0.0*deg2rad) ); //  north
  latA=50*deg2rad, latB=50*deg2rad, lonA=5*deg2rad, lonB=6*deg2rad;
  CHECK( bearing(latA,lonA,latB,lonB) == Approx(89.617*deg2rad) ); // east
  latA=51*deg2rad, latB=50*deg2rad, lonA=5*deg2rad, lonB=5*deg2rad;
  CHECK( bearing(latA,lonA,latB,lonB) == Approx(180.0*deg2rad) ); // south
  latA=50*deg2rad, latB=50*deg2rad, lonA=6*deg2rad, lonB=5*deg2rad;
  CHECK( bearing(latA,lonA,latB,lonB) == Approx(-89.617*deg2rad) ); // west
  // equator
  latA=0*deg2rad, latB=0*deg2rad, lonA=5*deg2rad, lonB=6*deg2rad;
  CHECK( bearing(latA,lonA,latB,lonB) == Approx(90*deg2rad) ); // east
  latA=0*deg2rad, latB=0*deg2rad, lonA=6*deg2rad, lonB=5*deg2rad;
  CHECK( bearing(latA,lonA,latB,lonB) == Approx(-90*deg2rad) ); // west
  // cross hemispheres
  latA=0*deg2rad, latB=0*deg2rad, lonA=-5*deg2rad, lonB=5*deg2rad;
  CHECK( bearing(latA,lonA,latB,lonB) == Approx(90.0*deg2rad) ); // east
  latA=0*deg2rad, latB=0*deg2rad, lonA=5*deg2rad, lonB=-5*deg2rad;
  CHECK( bearing(latA,lonA,latB,lonB) == Approx(-90.0*deg2rad) ); // west
  latA=0*deg2rad, latB=0*deg2rad, lonA=170*deg2rad, lonB=-170*deg2rad;
  CHECK( bearing(latA,lonA,latB,lonB) == Approx(90.0*deg2rad) ); // east
  latA=0*deg2rad, latB=0*deg2rad, lonA=-170*deg2rad, lonB=170*deg2rad;
  CHECK( bearing(latA,lonA,latB,lonB) == Approx(-90.0*deg2rad) ); // west
  // cross poles
  latA=85*deg2rad, latB=85*deg2rad, lonA=10*deg2rad, lonB=-170*deg2rad;
  CHECK( bearing(latA,lonA,latB,lonB) == Approx(0.0*deg2rad) ); // north
  latA=85*deg2rad, latB=85*deg2rad, lonA=-170*deg2rad, lonB=10*deg2rad;
  CHECK( bearing(latA,lonA,latB,lonB) == Approx(0.0*deg2rad) ); // still north
  latA=-85*deg2rad, latB=-85*deg2rad, lonA=10*deg2rad, lonB=-170.001*deg2rad;
  CHECK( bearing(latA,lonA,latB,lonB) == Approx(180.0*deg2rad) ); // south
  latA=-85*deg2rad, latB=-85*deg2rad, lonA=-170*deg2rad, lonB=9.999*deg2rad;
  CHECK( bearing(latA,lonA,latB,lonB) == Approx(180.0*deg2rad) ); // south
}

TEST_CASE( "destination", "destination (from location+dist+bearing)" ) {
  double lat(50*deg2rad), lon(6*deg2rad), dist(111195), bearing(0);
  CHECK( destination(lat,lon,dist,bearing).first == Approx(51*deg2rad) ); //  north
  CHECK( destination(lat,lon,dist,bearing).second == Approx(6*deg2rad) ); //  north
  lat=0*deg2rad, lon=6*deg2rad, dist=111195, bearing=90*deg2rad;
  CHECK( destination(lat,lon,dist,bearing).first == Approx(0*deg2rad) ); //  east
  CHECK( destination(lat,lon,dist,bearing).second == Approx(7*deg2rad) ); //  east
  lat=-1*deg2rad, lon=6*deg2rad, dist=111195, bearing=180*deg2rad;
  CHECK( destination(lat,lon,dist,bearing).first == Approx(-2*deg2rad) ); //  east
  CHECK( destination(lat,lon,dist,bearing).second == Approx(6*deg2rad) ); //  east
  lat=0*deg2rad, lon=-2*deg2rad, dist=111195, bearing=-90*deg2rad;
  CHECK( destination(lat,lon,dist,bearing).first == Approx(0*deg2rad) ); //  east
  CHECK( destination(lat,lon,dist,bearing).second == Approx(-3*deg2rad) ); //  east
}


TEST_CASE("rgb2hsl","rgb2hsl") {
  CHECK( rgb2hsl(0,0,0) == vector<double>({0.0, 0.0, 0.0}) );
  CHECK( rgb2hsl(20,20,20)[0] == Approx(0) );
  CHECK( rgb2hsl(20,20,20)[1] == Approx(0) );
  CHECK( rgb2hsl(20,20,20)[2] == Approx(0.078431) );
  CHECK( rgb2hsl(255,255,255) == vector<double>({0.0, 0.0, 1.0}) );
  CHECK( rgb2hsl(50,150,250)[0] == Approx(0.583333) ); //h
  CHECK( rgb2hsl(50,150,250)[1] == Approx(0.952380) ); //s
  CHECK( rgb2hsl(50,150,250)[2] == Approx(0.588235) ); //l
}

TEST_CASE("hsv2rgb","hsv2rgb"){
  CHECK(hsv2rgb(0,0,0) == vector<int>({int(0*255),int(0*255),int(0*255)}));
  CHECK(hsv2rgb(1,0,0) == vector<int>({int(0*255),int(0*255),int(0*255)}));
  CHECK(hsv2rgb(1,1,1) == vector<int>({int(1*255),int(0*255),int(0*255)}));
  CHECK(hsv2rgb(0.2,0.3,0.4) == vector<int>({int(0.376*255),int(0.4*255),int(0.28*255)}));
}

TEST_CASE("rgb2hsv","rgb2hsv") {
  CHECK( rgb2hsv(0,0,0) == vector<double>({0.0, 0.0, 0.0}) );
  CHECK( rgb2hsl(20,20,20)[0] == Approx(0) );
  CHECK( rgb2hsl(20,20,20)[1] == Approx(0) );
  CHECK( rgb2hsl(20,20,20)[2] == Approx(0.078431) );
  CHECK( rgb2hsv(255,255,255) == vector<double>({0.0, 0.0, 1.0}) );
  CHECK( rgb2hsv(50,150,250)[0] == Approx(0.583333) ); //h
  CHECK( rgb2hsv(50,150,250)[1] == Approx(0.8) ); //s
  CHECK( rgb2hsv(50,150,250)[2] == Approx(0.980392) ); //v
}

