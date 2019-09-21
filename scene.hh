#ifndef SCENE_HH
#define SCENE_HH

#include <algorithm>
#include <cmath>
#include <cstring>
#include <fstream>  // for ofstream
#include <unordered_map>
#include <utility>  // for pair

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
cout << "required_tiles: " << required_tiles << endl;
    for(auto it=required_tiles.begin(), to=required_tiles.end(); it!=to; it++){
      const int ref_lat = it->first, ref_lon = it->second;
      string path("hgt");
      string fn(string(ref_lat<0?"S":"N") + to_string_fixedwidth(abs(ref_lat),2) +
                string(ref_lon<0?"W":"E") + to_string_fixedwidth(abs(ref_lon),3) + ".hgt");
      bool source_found = false;
      std::unordered_map<string, string> folder = {{"srtm1","SRTM1v3.0"}, {"srtm3","SRTM3v3.0"}, {"view1","VIEW1"}, {"view3","VIEW3"}};
      for(vector<string>::const_iterator sit=source.begin(), sot=source.end(); sit!=sot; sit++){
        string fn_full = path + "/" + folder[*sit] + "/" + fn;
//cout << fn_full << endl;
        if(file_accessable(fn_full)){
          // get tiles, add them
          int tile_size = 0;
          try{
            tile_size = 3600/stoi(string(&sit->back())) + 1;
            // tile_size = 3600/int(sit->back()-'0') + 1; // also works ... chars are horrible ...
          }
          catch (const std::invalid_argument& ia) {
            cerr << "Invalid argument: " << ia.what() << endl; }
          catch (const std::out_of_range& oor) {
            std::cerr << "Out of Range error: " << oor.what() << endl; }
          cout << "trying to read: " << fn_full << " with dimension " << tile_size << " ..." << flush;
          char const * FILENAME = fn_full.c_str();
          tile<int16_t> A(tile<int16_t>(FILENAME, tile_size, ref_lat, ref_lon));
          add_tile(A);
          cout << " done" << endl;
          source_found = true;
          break; // add only one version of each tile
        }
      }
      if(!source_found) cerr << " no source for "+fn+" found, ignoring it" << endl;
    }
    if(z_standpoint == -1){
       // FIXME the case when points from neighbouring tiles are required
       auto it = find_if(tiles.begin(), tiles.end(),
                         [&](const pair<tile<double>,tile<double>> & p) {
                         return (p.first.get_lat() == floor(lat_standpoint*rad2deg)) && (p.first.get_lon() == floor(lon_standpoint*rad2deg));}
                 );
       z_standpoint = (it->first).interpolate(lat_standpoint*rad2deg, lon_standpoint*rad2deg) + 10;
       cout << "overwriting the elevation: " << z_standpoint << endl;
    }
    cout << "scene constructed" << endl;
    debug.close();
  }

  // lat, lon
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
