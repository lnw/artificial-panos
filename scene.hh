#ifndef SCENE_HH
#define SCENE_HH

#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "auxiliary.hh"
#include "geometry.hh"
#include "array2D.hh"
#include "tile.hh"

using namespace std;

// everything about the depicted landscape that has nothing to do with pixels yet
class scene {
public:
  double lat_standpoint, lon_standpoint, z_standpoint; // [rad], [rad], [m]
  double view_dir, view_width, view_height, view_dist; // [rad], [rad], [rad], [m]
  vector<pair<tile<double>,tile<double>>> tiles; // heights, distances

  scene(double lat, double lon, double z, double vdir, double vw, double vh, double vdist): lat_standpoint(lat), lon_standpoint(lon), z_standpoint(z), view_dir(vdir), view_width(vw), view_height(vh), view_dist(vdist) {
    ofstream debug("debug-render_scene", ofstream::out | ofstream::app);
    debug << "standpoint: " << lat_standpoint*rad2deg << ", " << lon_standpoint*rad2deg << endl;

    // determine which tiles to add

    // -- shoot rays in a bunch of directions, get the floor, done

    // get tiles
    // add them
    const int size=3601;
    int ref_lat = 49, ref_lon = 8;
    char const * FILENAME = "N49E008.hgt";
    tile<int16_t> A (tile<int16_t>(FILENAME, size, ref_lat, ref_lon));
    add_tile(A);
    ref_lat = 49, ref_lon = 9;
    FILENAME = "N49E009.hgt";
    A = tile<int16_t>(FILENAME, size, ref_lat, ref_lon);
    add_tile(A);
    cout << "scene constructed" << endl;
    debug.close();
  }

  template <typename T> void add_tile(const tile<T>& Tile){
    tile<double> D(Tile.get_distances(lat_standpoint, lon_standpoint));
    tile<double> H(Tile.curvature_adjusted_elevations(D));
    tiles.push_back(make_pair(H,D));
  }

};

#endif
