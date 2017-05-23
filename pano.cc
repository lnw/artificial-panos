
#include <fstream>
#include <iostream>
#include <string>

#include "auxiliary.hh"
#include "geometry.hh"
#include "array2D.hh"
#include "tile.hh"
#include "scene.hh"
#include "canvas.hh"
#include "mapitems.hh"

using namespace std;

int main(int ac, char **av) {


  // find which files are required

  // get those

  // read files into vector of 2D arrays, the order does not matter as they can be rendered independently

  // transform (lower at larger distance) and afterwards elevations are treated to be relative to a plane
  // viewfinder uses drop/m = 0.1695 m * (dist / miles)^2 to account for curvature and refraction
  // ... distance between two coordinates/z?
  // ... angles on sphere?

  // const double pos_lat(59.87*deg2rad), pos_lon(10.67*deg2rad), pos_z(130); // rad, rad, m
  const double pos_lat(49.4*deg2rad), pos_lon(8.6*deg2rad), pos_z(160); // rad, rad, m
  // const double pos_lat(47.3664*deg2rad), pos_lon(8.5413*deg2rad), pos_z(408); // rad, rad, m
  const double view_direction(0*deg2rad); // [rad], east is 0
  const double view_width(270*deg2rad); // [rad]
  const double view_height(15*deg2rad); // [rad]
  const double range(80000); // [m]
  scene S(pos_lat, pos_lon, pos_z, view_direction, view_width, view_height, range);
//  cout << S.tiles[0].first << endl;
//  cout << S.tiles[0].second << endl;
//  cout << S.tiles[1].first << endl;
//  cout << S.tiles[1].second << endl;
//  cerr << "enough" << endl;


  char const * filename = "out.png";
  const int view_x(10000), view_y(1000); // pixels
  canvas V(filename, view_x, view_y);
  //V.render_test();

  // V.write_pixel_zb(5,5,500,   0,255,0,255);
  // V.write_pixel_zb(15,5,500,  0,255,0,255);
  // V.write_pixel_zb(5,15,500,  0,255,0,255);
  // V.write_pixel_zb(15,15,500, 0,255,0,255);

  // V.write_triangle(200,200,     300.3,310.4, 600.1,210.2, 15000, 0,255,100,255);
  // V.write_triangle(200,200,     300.3,310.4, 230.5,510.6, 15000, 0,100,255,255);
  // V.write_triangle(600.1,210.2, 300.3,310.4, 230.5,510.6, 15000, 0,200,255,255);

  V.bucket_fill(10,10,10,255);
  V.render_scene(S);
//  cout << V.zbuffer << endl;

  // V.write_tick_top(200, 30, 2, 1, 150,50,50,255);

  // iterate over landscape squares, maintain z-buffer
  // possibly transform z axis of picture:  viewfinder compresses forground and stretches region around horizon by 130%-200%
  // a) first paint each pixel according to the distance
  // b) hue according to distance, darker for surfaces that almost contain the location of the viewer


  return 0;
}

