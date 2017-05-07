
#include <cmath>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "auxiliary.hh"
#include "tile.hh"
#include "scene.hh"
#include "geometry.hh"

using namespace std;

int main(int ac, char **av) {

  const double deg2rad = M_PI/180;
  const double rad2deg = 180/M_PI;


  const double scene_direction = 270*deg2rad; // angle in rad, east is 0
  const double scene_width = 60*deg2rad; // angle in rad
  const double range = 50000; // in m

  const int view_width = 1000; // pixels
  const int view_height = 500; // pixels


  cout << scene_direction << endl;
  cout << scene_width << endl;
  cout << range << endl;



  // find which files are required

  // get those

  // read files into vector of 2D arrays, the order does not matter as they can be rendered independently

  // transform (lower at larger distance) and afterwards elevations are treated to be from a plane
  // viewfinder uses drop/m = 0.1695 m * (dist / miles)^2 to account for curvature and refraction
  // ... distance between two coordinates/z?
  // ... angles on sphere?

  // calculate array of triples with phi/theta/dist for each elevated point

  // iterate over landscape squares, maintain z-buffer
  // possibly transform z axis of picture:  viewfinder compresses forground and stretches region around horizon by 130%-200%
  // a) first paint each pixel according to the distance
  // b) hue according to distance, darker for surfaces that almost contain the location of the viewer



  return 0;
}


