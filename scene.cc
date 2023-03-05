#include <algorithm>
#include <cmath>
#include <cstring>
#include <fstream>
#include <unordered_map>
#include <utility>

#include "scene.hh"
#include "tile.hh"


inline bool file_accessable(const std::string& fn) {
  std::ifstream f(fn.c_str());
  return f.good();
}

scene::scene(double lat, double lon, double z, double vdirh, double vw, double vdirv, double vh, double vdist, const std::vector<std::string>& _source): lat_standpoint(lat), lon_standpoint(lon), z_standpoint(z), view_dir_h(vdirh), view_width(vw), view_dir_v(vdirv), view_height(vh), view_range(vdist), source(_source) {
  std::ofstream debug("debug-render_scene", std::ofstream::out | std::ofstream::app);
  debug << "standpoint: " << lat_standpoint * rad2deg << ", " << lon_standpoint * rad2deg << std::endl;

  // determine which tiles to add
  // sample a bunch of points, include the respective tiles
  std::vector<std::pair<int, int>> required_tiles = determine_required_tiles_v(view_width, view_range, view_dir_h, lat_standpoint, lon_standpoint);
  std::cout << "required_tiles: " << required_tiles << std::endl;
#pragma omp parallel for shared(tiles)
  for (auto it = required_tiles.begin(); it != required_tiles.end(); it++) {
    const int ref_lat = it->first,
              ref_lon = it->second;
    std::string path("hgt");
    std::string fn(std::string(ref_lat < 0 ? "S" : "N") + to_stringish_fixedwidth<std::string>(std::abs(ref_lat), 2) +
                   std::string(ref_lon < 0 ? "W" : "E") + to_stringish_fixedwidth<std::string>(std::abs(ref_lon), 3) + ".hgt");
    bool source_found = false;
    std::unordered_map<std::string, std::string> folder = {{"srtm1", "SRTM1v3.0"}, {"srtm3", "SRTM3v3.0"}, {"view1", "VIEW1"}, {"view3", "VIEW3"}};
    for (auto sit = source.begin(), sot = source.end(); sit != sot; sit++) {
      std::string fn_full = path + "/" + folder[*sit] + "/" + fn;
      //std::cout << fn_full << std::endl;
      if (file_accessable(fn_full)) {
        const auto t0 = std::chrono::high_resolution_clock::now();

        // get tiles, add them
        int tile_size = 0;
        try {
          tile_size = 3600 / stoi(std::string(&sit->back())) + 1;
          // tile_size = 3600/int(sit->back()-'0') + 1; // also works ... chars are horrible ...
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
        std::cout << "trying to read: " << fn_full << " with dimension " << tile_size << " ..." << std::endl; // flush;
        char const* FILENAME = fn_full.c_str();
        tile<int16_t> A(tile<int16_t>(FILENAME, tile_size, ref_lat, ref_lon));
        // auto t2 = std::chrono::high_resolution_clock::now();
        // fp_ms = t2 - t1;
        // std::cout << "  reading " << std::string(FILENAME) << " took " << fp_ms.count() << " ms" << std::endl;
        add_tile(A);
        // std::cout << " done" << std::endl;
        source_found = true;

        auto t3 = std::chrono::high_resolution_clock::now();
        // std::chrono::duration<double, std::milli>  fp_ms_2 = t3 - t2;
        std::chrono::duration<double, std::milli> fp_ms_tot = t3 - t0;
        // std::cout << "  adding tile took " << fp_ms_2.count() << " ms" << std::endl;
        std::cout << "  reading + processing tile " << std::string(FILENAME) << " took " << fp_ms_tot.count() << " ms" << std::endl;

        break; // add only one version of each tile
      }
    }
    if (!source_found)
      std::cerr << " no source for " + fn + " found, ignoring it" << std::endl;
  }
  if (z_standpoint == -1) {
    const double z_offset = 10; // asssume we are floating in midair to avoid artefacts
    // FIXME the case when points from neighbouring tiles are required
    // find the tile in which we are standing
    auto it = find_if(tiles.begin(), tiles.end(),
                      [&](const std::pair<tile<double>, tile<double>>& p) { return (p.first.lat() == floor(lat_standpoint * rad2deg)) && (p.first.lon() == floor(lon_standpoint * rad2deg)); });
    z_standpoint = (it->first).interpolate(lat_standpoint * rad2deg, lon_standpoint * rad2deg) + z_offset;
    std::cout << "overwriting the elevation: " << z_standpoint << std::endl;
  }
  std::cout << "scene constructed" << std::endl;
  debug.close();
}
