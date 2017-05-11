
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


  // find which files are required

  // get those

  // read files into vector of 2D arrays, the order does not matter as they can be rendered independently

  // transform (lower at larger distance) and afterwards elevations are treated to be relative to a plane
  // viewfinder uses drop/m = 0.1695 m * (dist / miles)^2 to account for curvature and refraction
  // ... distance between two coordinates/z?
  // ... angles on sphere?

  const double pos_lat(49.4*deg2rad_const), pos_lon(8.6*deg2rad_const), pos_z(200); // rad, rad, m
  const double scene_direction(0*deg2rad_const); // [rad], east is 0
  const double scene_width(120*deg2rad_const); // [rad]
  const double scene_height(30*deg2rad_const); // [rad]
  const double range(50000); // [m]
  scene S(pos_lat, pos_lon, pos_z, scene_direction, scene_width, scene_height, range);

  // calculate array of triples with phi/theta/dist for each elevated point

  char const * filename = "out.png";
  const int view_x(3000), view_y(800); // pixels
  canvas V(filename, view_x, view_y);
  V.render_test();

  V.write_pixel_zb(5,5,500,   0,255,0,255);
  V.write_pixel_zb(15,5,500,  0,255,0,255);
  V.write_pixel_zb(5,15,500,  0,255,0,255);
  V.write_pixel_zb(15,15,500, 0,255,0,255);

  V.write_triangle(200,200,     300.3,310.4, 600.1,210.2, 500,   0,255,100,255);
  V.write_triangle(200,200,     300.3,310.4, 230.5,510.6, 500,   0,100,255,255);
  V.write_triangle(600.1,210.2, 300.3,310.4, 230.5,510.6, 500.7, 0,200,255,255);

  V.render_scene(S);
//  cout << V.zbuffer << endl;

  // iterate over landscape squares, maintain z-buffer
  // possibly transform z axis of picture:  viewfinder compresses forground and stretches region around horizon by 130%-200%
  // a) first paint each pixel according to the distance
  // b) hue according to distance, darker for surfaces that almost contain the location of the viewer



  return 0;
}

