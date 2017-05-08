#ifndef SCENE_HH
#define SCENE_HH

#include <vector>
#include <iostream>
#include <cmath>

#include "auxiliary.hh"
#include "geometry.hh"
#include "array2D.hh"
#include "tile.hh"

using namespace std;

// everything about the depicted landscape that has nothing to do with pixels yet
class scene {
public:
  double lat_standpoint, lon_standpoint, z_standpoint; // [rad], [rad], [m]
  double view_dir, view_width, view_dist; // [rad], [rad], [km]
  vector<pair<tile<double>,tile<double>>> tiles; // heights, distances

  scene(double lat, double lon, double z, double vdir, double vw, double vdist): lat_standpoint(lat), lon_standpoint(lon), z_standpoint(z), view_dir(vdir), view_width(vw), view_dist(vdist) {
    // determine which tiles to add
    // get tiles
    // add them
    const int size=3601;
    tile<int16_t> A (tile<int16_t>("N59E006.hgt",size,59,6));
    add_tile(A);
  };

  template <typename T> void add_tile(tile<T> Tile){
    tile<double> D(Tile.get_distances(lat_standpoint, lon_standpoint));
    tile<double> H(Tile.adjust_curvature(D));
    tiles.push_back(make_pair(D,H));
  }
  
  
  

};

#endif
