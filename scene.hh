#ifndef SCENE_HH
#define SCENE_HH

#include <math.h>   // for M_PI, fmod
#include <fstream>  // for ofstream
#include <utility>  // for pair
#include <algorithm>
#include <string>

#include "tile.hh"  // for tile

using namespace std;

inline bool file_accessable(const string& fn) {
  ifstream f(fn.c_str());
  return f.good();
}

// everything about the depicted landscape that has nothing to do with pixels yet
class scene {
public:
  double lat_standpoint, lon_standpoint, z_standpoint; // [rad], [rad], [m]
  double view_dir_h, view_width, view_dir_v, view_height; // [rad], [rad], [rad], [rad]
  double view_range; // [m]
  vector<string> source; // list of subset of view1, view3, srtm1, srtm3 in some order: these are considered as source
  vector<pair<tile<double>,tile<double>>> tiles; // heights, distances

  scene(double lat, double lon, double z, double vdirh, double vw, double vdirv, double vh, double vdist, vector<string> _source): lat_standpoint(lat), lon_standpoint(lon), z_standpoint(z), view_dir_h(vdirh), view_width(vw), view_dir_v(vdirv), view_height(vh), view_range(vdist), source(_source) {
    ofstream debug("debug-render_scene", ofstream::out | ofstream::app);
    debug << "standpoint: " << lat_standpoint*rad2deg << ", " << lon_standpoint*rad2deg << endl;

    // determine which tiles to add
    // sample a bunch of points, include the respective tiles
    set<pair<int,int>> required_tiles = determine_required_tiles(view_width, view_range, view_dir_h, lat_standpoint, lon_standpoint);
    for(auto it=required_tiles.begin(), to=required_tiles.end(); it!=to; it++){
      for(vector<string>::const_iterator sit=source.begin(), sot=source.end(); sit!=sot; sit++){
        const int ref_lat = it->first, ref_lon = it->second;
        string path("hgt");
        string fn(string(ref_lat<0?"S":"N") + to_string_fixedwidth(abs(ref_lat),2) +
                  string(ref_lon<0?"W":"E") + to_string_fixedwidth(abs(ref_lon),3) + ".hgt");
        fn = path + "/" + *sit + "/" + fn;
        if(file_accessable(fn)){
          // get tiles, add them
          int tile_size=0; 
          try{
            tile_size = 3600/stoi(string(&sit->back())) + 1;
            // tile_size = 3600/int(sit->back()-'0') + 1; // also works ... chars are horrible ...
          }
          catch (const std::invalid_argument& ia) {
            cerr << "Invalid argument: " << ia.what() << endl; }
          catch (const std::out_of_range& oor) {
            std::cerr << "Out of Range error: " << oor.what() << endl; }
          cout << "trying to read: " << fn << " with dimension " << tile_size << " ..." << flush;
          char const * FILENAME = fn.c_str();
          tile<int16_t> A(tile<int16_t>(FILENAME, tile_size, ref_lat, ref_lon));
          add_tile(A);
          cout << " done" << endl;
          break; // add only one version of each tile
        }
      }
    }
    if(z_standpoint == -1){
       // FIXME the case when points from neighbouring tiles are required
       auto it = find_if(tiles.begin(), tiles.end(),
                         [&](const pair<tile<double>,tile<double>> & p) {
                         return (p.first.lat == floor(lat_standpoint*rad2deg)) && (p.first.lon == floor(lon_standpoint*rad2deg));}
                 );
       z_standpoint = (it->first).interpolate(lat_standpoint*rad2deg, lon_standpoint*rad2deg) + 10;
       cout << "overwriting the elevation: " << z_standpoint << endl;
    }
    cout << "scene constructed" << endl;
    debug.close();
  }

  static set<pair<int,int>> determine_required_tiles(const double view_width, const double view_range, const double view_dir_h, const double lat_standpoint, const double lon_standpoint){
    set<pair<int,int>> rt;
    for(int i=0; i<10; i++){
      const int n_ray=20;
      const double dist = i* view_range/9;
      for(int j=0; j<n_ray; j++){
        const double bearing = fmod(-view_dir_h - view_width/2 + M_PI/2 + j*view_width/(n_ray-1) + 3*M_PI, 2*M_PI) - M_PI;
        pair<double,double> dest = destination(lat_standpoint, lon_standpoint, dist, bearing);
        rt.insert(make_pair(floor(dest.first*rad2deg), floor(dest.second*rad2deg)));
      }
    }
    return rt;
  }

  template <typename T> void add_tile(const tile<T>& Tile){
    tile<double> D(Tile.get_distances(lat_standpoint, lon_standpoint));
    tile<double> H(Tile.curvature_adjusted_elevations(D));
    tiles.push_back(make_pair(H,D));
  }

};

#endif
