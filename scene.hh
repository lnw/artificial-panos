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
class scene {
public:
  LatLon<double, Unit::rad> standpoint;
  double z_standpoint;                                      // [m]
  double view_dir_h, view_width, view_dir_v, view_height;   // [rad], [rad], [rad], [rad]
  double view_range;                                        // [m]
  std::vector<elevation_source> sources;                    // list of subset of view1, view3, srtm1, srtm3 in some order: these are considered as source
  std::vector<std::pair<tile<double>, tile<double>>> tiles; // heights, distances

  scene(LatLon<double, Unit::rad> standpoint, double z, double vdirh, double vw, double vdirv, double vh, double vdist, const std::vector<elevation_source>& _sources);

  static std::set<LatLon<int64_t, Unit::deg>> determine_required_tiles(const double view_width, const double view_range, const double view_dir_h, const LatLon<double, Unit::rad> standpoint) {
    const int samples_per_ray = 10;
    std::set<LatLon<int64_t, Unit::deg>> rt;
    for (int i = 0; i < samples_per_ray; i++) {
      const int n_ray = 20;
      const double dist = i * view_range / (samples_per_ray - 1);
      for (int j = 0; j < n_ray; j++) {
        const double bearing = std::fmod(-view_dir_h - view_width / 2 + M_PI / 2 + j * view_width / (n_ray - 1) + 3 * M_PI, 2 * M_PI) - M_PI;
        const LatLon<double, Unit::rad> dest = destination(standpoint, dist, bearing);
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

  std::vector<std::pair<tile<double>, tile<double>>> read_elevation_data(const std::vector<LatLon<int64_t, Unit::deg>>& required_tiles) const;
  double elevation_at_standpoint() const;
};
