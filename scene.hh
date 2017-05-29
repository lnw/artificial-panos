#ifndef SCENE_HH
#define SCENE_HH

#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <set>

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
    // sample a bunch of points, include the respective tiles
    set<pair<int,int>> required_tiles;
    for(int i=0; i<10; i++){
      const double dist = i* view_dist/9;
      for(int j=0; j<10; j++){
        const double bearing = fmod(-view_dir - view_width/2 + M_PI/2 + j*view_width/9 + 3*M_PI, 2*M_PI) - M_PI;
        pair<double,double> dest = destination(lat_standpoint, lon_standpoint, dist, bearing);
        required_tiles.insert(make_pair(floor(dest.first*rad2deg), floor(dest.second*rad2deg)));
      }
    }
    const int size=3601;
    cout << "required tiles: " << required_tiles << endl;
    for(auto it=required_tiles.begin(), to=required_tiles.end(); it!=to; it++){
      // get tiles, add them
      const int ref_lat = it->first, ref_lon = it->second;
      string fn(string(ref_lat<0?"S":"N") + to_string_fixedwidth(abs(ref_lat),2) +
                string(ref_lon<0?"W":"E") + to_string_fixedwidth(abs(ref_lon),3) + ".hgt");
      cout << "trying to read: " << fn << " ..." << flush;
      char const * FILENAME = fn.c_str();
      tile<int16_t> A (tile<int16_t>(FILENAME, size, ref_lat, ref_lon));
      add_tile(A);
      cout << " done" << endl;
    } 
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
