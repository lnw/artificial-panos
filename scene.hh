#pragma once

#include <algorithm>
#include <cmath>
#include <fstream>
#include <utility>

#include "latlon.hh"
#include "tile.hh"


// everything about the depicted landscape that has nothing to do with pixels yet
class scene {
public:
  LatLon<double, Unit::rad> standpoint;
  double z_standpoint;                                      // [m]
  double view_dir_h, view_width, view_dir_v, view_height;   // [rad], [rad], [rad], [rad]
  double view_range;                                        // [m]
  std::vector<std::string> source;                          // list of subset of view1, view3, srtm1, srtm3 in some order: these are considered as source
  std::vector<std::pair<tile<double>, tile<double>>> tiles; // heights, distances

  scene(LatLon<double, Unit::rad> standpoint, double z, double vdirh, double vw, double vdirv, double vh, double vdist, const std::vector<std::string>& _source);

  static std::set<LatLon<int64_t, Unit::deg>> determine_required_tiles(const double view_width, const double view_range, const double view_dir_h, const LatLon<double, Unit::rad> standpoint) {
    const int samples_per_ray = 10;
    std::set<LatLon<int64_t, Unit::deg>> rt;
    for (int i = 0; i < samples_per_ray; i++) {
      const int n_ray = 20;
      const double dist = i * view_range / (samples_per_ray - 1);
      for (int j = 0; j < n_ray; j++) {
        const double bearing = std::fmod(-view_dir_h - view_width / 2 + M_PI / 2 + j * view_width / (n_ray - 1) + 3 * M_PI, 2 * M_PI) - M_PI;
        LatLon<double, Unit::rad> dest = destination(standpoint, dist, bearing);
        rt.insert(floor(dest.to_deg()));
      }
    }
    return rt;
  }

  // wrapper that returns a vector because OMP-for loops require a random access iterator
  static std::vector<LatLon<int64_t, Unit::deg>> determine_required_tiles_v(const double view_width, const double view_range, const double view_dir_h, const LatLon<double, Unit::rad> standpoint) {
    const std::set<LatLon<int64_t, Unit::deg>> rt = determine_required_tiles(view_width, view_range, view_dir_h, standpoint);
    const std::vector<LatLon<int64_t, Unit::deg>> rt_v(rt.begin(), rt.end());
    return rt_v;
  }

  template <typename T>
  void add_tile(const tile<T>& Tile) {
    // auto t0 = std::chrono::high_resolution_clock::now();
    tile<double> D(Tile.get_distances(standpoint));
    // auto t1 = std::chrono::high_resolution_clock::now();
    // std::chrono::duration<double, std::milli> fp_ms = t1 - t0;
    // cout << "    distances took " << fp_ms.count() << " ms" << endl;
    tile<double> H(Tile.curvature_adjusted_elevations(D));
    // auto t2 = std::chrono::high_resolution_clock::now();
    // fp_ms = t2 - t1;
    // cout << "    curvature took " << fp_ms.count() << " ms" << endl;
    tiles.push_back(std::make_pair(H, D));
    // auto t3 = std::chrono::high_resolution_clock::now();
    // fp_ms = t3 - t2;
    // cout << "    pair took " << fp_ms.count() << " ms" << endl;
  }
};
