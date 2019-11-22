#ifndef SCENE_HH
#define SCENE_HH

#include <algorithm>
#include <cmath>
#include <cstring>
#include <fstream> // for ofstream
#include <unordered_map>
#include <utility> // for pair

#include "tile.hh"

using namespace std;


// everything about the depicted landscape that has nothing to do with pixels yet
class scene {
public:
  double lat_standpoint, lon_standpoint, z_standpoint;    // [rad], [rad], [m]
  double view_dir_h, view_width, view_dir_v, view_height; // [rad], [rad], [rad], [rad]
  double view_range;                                      // [m]
  vector<string> source;                                  // list of subset of view1, view3, srtm1, srtm3 in some order: these are considered as source
  vector<pair<tile<double>, tile<double>>> tiles;         // heights, distances

  scene(double lat, double lon, double z, double vdirh, double vw, double vdirv, double vh, double vdist, vector<string> _source);

  // lat, lon
  static set<pair<int, int>> determine_required_tiles(const double view_width, const double view_range, const double view_dir_h, const double lat_standpoint, const double lon_standpoint) {
    const int samples_per_ray = 10;
    set<pair<int, int>> rt;
    for (int i = 0; i < samples_per_ray; i++) {
      const int n_ray = 20;
      const double dist = i * view_range / (samples_per_ray - 1);
      for (int j = 0; j < n_ray; j++) {
        const double bearing = fmod(-view_dir_h - view_width / 2 + M_PI / 2 + j * view_width / (n_ray - 1) + 3 * M_PI, 2 * M_PI) - M_PI;
        pair<double, double> dest = destination(lat_standpoint, lon_standpoint, dist, bearing);
        rt.insert(make_pair(floor(dest.first * rad2deg), floor(dest.second * rad2deg)));
      }
    }
    return rt;
  }

  // wrapper that returns a vector because OMP-for loops require a random access iterator
  static vector<pair<int, int>> determine_required_tiles_v(const double view_width, const double view_range, const double view_dir_h, const double lat_standpoint, const double lon_standpoint) {
    const set<pair<int, int>> rt = determine_required_tiles(view_width, view_range, view_dir_h, lat_standpoint, lon_standpoint);
    const vector<pair<int, int>> rt_v(rt.begin(), rt.end());
    return rt_v;
  }

  template <typename T>
  void add_tile(const tile<T>& Tile) {
    // auto t0 = std::chrono::high_resolution_clock::now();
    tile<double> D(Tile.get_distances(lat_standpoint, lon_standpoint));
    // auto t1 = std::chrono::high_resolution_clock::now();
    // std::chrono::duration<double, std::milli> fp_ms = t1 - t0;
    // cout << "    distances took " << fp_ms.count() << " ms" << endl;
    tile<double> H(Tile.curvature_adjusted_elevations(D));
    // auto t2 = std::chrono::high_resolution_clock::now();
    // fp_ms = t2 - t1;
    // cout << "    curvature took " << fp_ms.count() << " ms" << endl;
    tiles.push_back(make_pair(H, D));
    // auto t3 = std::chrono::high_resolution_clock::now();
    // fp_ms = t3 - t2;
    // cout << "    pair took " << fp_ms.count() << " ms" << endl;
  }
};

#endif
