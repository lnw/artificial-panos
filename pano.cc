
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "auxiliary.hh"
#include "geometry.hh"
#include "array2D.hh"
#include "tile.hh"
#include "scene.hh"
#include "canvas.hh"

using namespace std;

int main(int ac, char **av) {

  const double deg2rad_const = M_PI/180;
  const double rad2deg_const = 180/M_PI;


  // find which files are required

  // get those

  // read files into vector of 2D arrays, the order does not matter as they can be rendered independently

  // transform (lower at larger distance) and afterwards elevations are treated to be from a plane
  // viewfinder uses drop/m = 0.1695 m * (dist / miles)^2 to account for curvature and refraction
  // ... distance between two coordinates/z?
  // ... angles on sphere?

  const double pos_lat(49.4*deg2rad_const), pos_lon(8.6*deg2rad_const), pos_z(200); // rad, rad, m
  const double scene_direction(0*deg2rad_const); // [rad], east is 0
  const double scene_width(120*deg2rad_const); // [rad]
  const double range(50); // [km]
  scene S(pos_lat, pos_lon, pos_z, scene_direction, scene_width, range);

  // calculate array of triples with phi/theta/dist for each elevated point

  char const * filename = "out.png";
  const int view_x(300), view_y(80); // pixels
  canvas V(filename, view_x, view_y);
  V.render_test();
  V.write_pixel_zb(5,5,500, 0,255,0,255);
  V.write_pixel_zb(15,5,500, 0,255,0,255);
  V.write_pixel_zb(5,15,500, 0,255,0,255);
  V.write_pixel_zb(15,15,500, 0,255,0,255);
  V.write_triangle(20,20,30,30,50,23,500, 0,255,0,255);
  V.write_triangle(20,21,30.3,31.4,23.5,51.6,500, 0,100,255,255);

  V.write_triangle(60.1,21.2,30.3,31.4,23.5,51.6,500.7, 0,100,255,255);
//  cout << V.zbuffer << endl;

  // iterate over landscape squares, maintain z-buffer
  // possibly transform z axis of picture:  viewfinder compresses forground and stretches region around horizon by 130%-200%
  // a) first paint each pixel according to the distance
  // b) hue according to distance, darker for surfaces that almost contain the location of the viewer



  return 0;
}

