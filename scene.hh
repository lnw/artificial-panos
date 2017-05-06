#ifndef SCENE_HH
#define SCENE_HH

#include <vector>
#include <iostream>
#include <cmath>

#include "array2D.hh"
#include "tile.hh"

using namespace std;

// everything about the depicted lalndscape that has nothing to do with pixels yet
class scene {

  double lat_standpoint, lon_standpoint, z_standpoint;
  vector<tile<double>> tiles;
  vector<tile<double>> heights;

  
public:

};

#endif
