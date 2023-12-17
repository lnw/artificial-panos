#pragma once

#include <algorithm>
#include <cmath>
#include <filesystem>
#include <utility>

#include "latlon.hh"
#include "tile.hh"

namespace fs = std::filesystem;

enum class elevation_source { srtm1 = 0,
                              srtm3,
                              view1,
                              view3 };
inline static std::vector<std::string> elevation_source_name = {"srtm1", "srtm3", "view1", "view3"};
inline static std::vector<fs::path> elevation_source_folder = {"SRTM1v3.0", "SRTM3v3.0", "VIEW1", "VIEW3"};
inline static std::vector<int> elevation_source_resolution = {1, 3, 1, 3};


// everything about the depicted landscape that has nothing to do with pixels yet
template <typename T>
class scene {
public:
  LatLon<T, Unit::rad> standpoint;
  T z_standpoint;                                    // [m]
  T view_dir_h, view_width, view_dir_v, view_height; // [rad], [rad], [rad], [rad]
  T view_range;                                      // [m]
  std::vector<elevation_source> sources;             // list of subset of view1, view3, srtm1, srtm3 in some order: these are considered as source
  std::vector<std::pair<tile<T>, tile<T>>> tiles;    // heights, distances

  scene(LatLon<T, Unit::rad> standpoint, T z, T vdirh, T vw, T vdirv, T vh, T vdist, const std::vector<elevation_source>& _sources);

  static std::set<LatLon<int64_t, Unit::deg>> determine_required_tiles(const T view_width, const T view_range, const T view_dir_h, const LatLon<T, Unit::rad> standpoint) {
    const int samples_per_ray = 10;
    std::set<LatLon<int64_t, Unit::deg>> rt;
    for (int i = 0; i < samples_per_ray; i++) {
      const int n_ray = 20;
      const T dist = i * view_range / (samples_per_ray - 1);
      for (int j = 0; j < n_ray; j++) {
        const T bearing = std::fmod(-view_dir_h - view_width / 2 + M_PI / 2 + j * view_width / (n_ray - 1) + 3 * M_PI, 2 * M_PI) - M_PI;
        const LatLon<T, Unit::rad> dest = destination(standpoint, dist, bearing);
        rt.insert(floor(dest.to_deg()));
      }
    }
    return rt;
  }

  // wrapper that returns a vector because OMP-for loops require a random access iterator
  static std::vector<LatLon<int64_t, Unit::deg>> determine_required_tiles_v(const T view_width, const T view_range, const T view_dir_h, const LatLon<T, Unit::rad> standpoint) {
    const std::set<LatLon<int64_t, Unit::deg>> rt = determine_required_tiles(view_width, view_range, view_dir_h, standpoint);
    const std::vector<LatLon<int64_t, Unit::deg>> rt_v(rt.begin(), rt.end());
    return rt_v;
  }

  std::vector<std::pair<tile<T>, tile<T>>> read_elevation_data(const std::vector<LatLon<int64_t, Unit::deg>>& required_tiles) const;
  T elevation_at_standpoint() const;
};
