#include <algorithm>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <ranges>
#include <utility>

#include "scene.hh"
#include "tile.hh"

namespace fs = std::filesystem;

inline bool file_accessable(const fs::path& fp) {
  if (!exists(fp))
    return false;
  std::ifstream f(fp.string().c_str());
  return f.good();
}

scene::scene(LatLon<double, Unit::rad> coords, double z, double vdirh, double vw, double vdirv, double vh, double vdist, const std::vector<elevation_source>& _sources): standpoint(coords), z_standpoint(z), view_dir_h(vdirh), view_width(vw), view_dir_v(vdirv), view_height(vh), view_range(vdist), sources(_sources) {
  const std::vector<LatLon<int64_t, Unit::deg>> required_tiles = determine_required_tiles_v(view_width, view_range, view_dir_h, standpoint);
  std::cout << "required_tiles: " << required_tiles << std::endl;
  tiles = read_elevation_data(required_tiles);
  if (z_standpoint == -1) {
    const double z_offset = 10; // assume we are floating in some metres above ground to avoid artefacts
    z_standpoint = elevation_at_standpoint() + z_offset;
    std::cout << "overwriting the elevation: " << z_standpoint << std::endl;
  }
}


std::vector<std::pair<tile<double>, tile<double>>> scene::read_elevation_data(const std::vector<LatLon<int64_t, Unit::deg>>& required_tiles) const {
  std::vector<std::pair<tile<double>, tile<double>>> res(required_tiles.size());
#pragma omp parallel for shared(res)
  for (const auto& [tile_index, required_tile] : std::ranges::views::enumerate(required_tiles)) {
    const auto [ref_lat, ref_lon] = required_tile;
    fs::path path("hgt");
    fs::path fn(std::string(ref_lat < 0 ? "S" : "N") + to_stringish_fixedwidth<std::string>(std::abs(ref_lat), 2) +
                std::string(ref_lon < 0 ? "W" : "E") + to_stringish_fixedwidth<std::string>(std::abs(ref_lon), 3) + ".hgt");
    bool source_found = false;
    for (const auto& source : sources) {
      fs::path filename_rel = path / elevation_source_folder[std::to_underlying(source)] / fn;
      // std::cout << fn_full << std::endl;
      if (!file_accessable(filename_rel))
        continue;

      const auto t0 = std::chrono::high_resolution_clock::now();

      // get tiles, add them
      int tile_size = 0;
      try {
        tile_size = 3600 / elevation_source_resolution[std::to_underlying(source)] + 1;
      }
      catch (const std::invalid_argument& ia) {
        std::cerr << "Invalid argument: " << ia.what() << std::endl;
      }
      catch (const std::out_of_range& oor) {
        std::cerr << "Out of Range error: " << oor.what() << std::endl;
      }
      // auto t1 = std::chrono::high_resolution_clock::now();
      // std::chrono::duration<double, std::milli> fp_ms = t1 - t0;
      // std::cout << "  preperation took " << fp_ms.count() << " ms" << std::endl;
      std::cout << "trying to read: " << filename_rel.string() << " with dimension " << tile_size << " ..." << std::endl; // flush;
      const tile<int16_t> A(filename_rel, tile_size, required_tile);
      // auto t2 = std::chrono::high_resolution_clock::now();
      // fp_ms = t2 - t1;
      // std::cout << "  reading " << std::string(FILENAME) << " took " << fp_ms.count() << " ms" << std::endl;
      // add_tile(A);
      tile<double> dists(A.get_distances(standpoint));
      tile<double> heights(A.curvature_adjusted_elevations(dists));
      res[tile_index] = std::make_pair(std::move(heights), std::move(dists));
      // std::cout << " done" << std::endl;
      source_found = true;

      auto t3 = std::chrono::high_resolution_clock::now();
      // std::chrono::duration<double, std::milli>  fp_ms_2 = t3 - t2;
      std::chrono::duration<double, std::milli> fp_ms_tot = t3 - t0;
      // std::cout << "  adding tile took " << fp_ms_2.count() << " ms" << std::endl;
      std::cout << "  reading + processing tile " << filename_rel.string() << " took " << fp_ms_tot.count() << " ms" << std::endl;

      break; // add only one version of each tile
    }
    if (!source_found)
      std::cerr << " no source for " + fn.string() + " found, ignoring it" << std::endl;
  }
  return res;
}


double scene::elevation_at_standpoint() const {
  auto it = std::find_if(tiles.begin(), tiles.end(),
                         [&](const auto& p) { return (p.first.lat() == std::floor(standpoint.to_deg().lat())) && (p.first.lon() == std::floor(standpoint.to_deg().lon())); });
  return (it->first).interpolate(standpoint.to_deg());
}
