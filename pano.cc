
#include <cmath>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "auxiliary.hh"
#include "geometry.hh"
#include "array2D.hh"
#include "tile.hh"
#include "scene.hh"
#include "view.hh"

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

  const double pos_lat(0.5), pos_lon(0.5), pos_z(200); // rad, rad, m
  const double scene_direction(270*deg2rad_const); // angle in rad, east is 0
  const double scene_width(60*deg2rad_const); // angle in rad
  const double range(50); // in km
  scene S(pos_lat, pos_lon, pos_z, scene_direction, scene_width, range);

  // calculate array of triples with phi/theta/dist for each elevated point

  const int view_x(3000), view_y(1000); // pixels
  view V(view_x, view_y, S);
  V.render();

  // iterate over landscape squares, maintain z-buffer
  // possibly transform z axis of picture:  viewfinder compresses forground and stretches region around horizon by 130%-200%
  // a) first paint each pixel according to the distance
  // b) hue according to distance, darker for surfaces that almost contain the location of the viewer



  return 0;
}


