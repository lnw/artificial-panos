
#include <algorithm>
#include <cmath>
#include <cstring>
#include <fstream> // for ofstream
#include <unordered_map>
#include <utility> // for pair

#include "scene.hh"
#include "tile.hh"

using namespace std;

inline bool file_accessable(const string& fn) {
  ifstream f(fn.c_str());
  return f.good();
}

scene::scene(double lat, double lon, double z, double vdirh, double vw, double vdirv, double vh, double vdist, const vector<string>& _source): lat_standpoint(lat), lon_standpoint(lon), z_standpoint(z), view_dir_h(vdirh), view_width(vw), view_dir_v(vdirv), view_height(vh), view_range(vdist), source(_source) {
  ofstream debug("debug-render_scene", ofstream::out | ofstream::app);
  debug << "standpoint: " << lat_standpoint * rad2deg << ", " << lon_standpoint * rad2deg << endl;

  // determine which tiles to add
  // sample a bunch of points, include the respective tiles
  vector<pair<int, int>> required_tiles = determine_required_tiles_v(view_width, view_range, view_dir_h, lat_standpoint, lon_standpoint);
  cout << "required_tiles: " << required_tiles << endl;
#pragma omp parallel for shared(tiles)
  for (auto it = required_tiles.begin(); it != required_tiles.end(); it++) {
    const int ref_lat = it->first,
              ref_lon = it->second;
    string path("hgt");
    string fn(string(ref_lat < 0 ? "S" : "N") + to_string_fixedwidth(std::abs(ref_lat), 2) +
              string(ref_lon < 0 ? "W" : "E") + to_string_fixedwidth(std::abs(ref_lon), 3) + ".hgt");
    bool source_found = false;
    std::unordered_map<string, string> folder = {{"srtm1", "SRTM1v3.0"}, {"srtm3", "SRTM3v3.0"}, {"view1", "VIEW1"}, {"view3", "VIEW3"}};
    for (auto sit = source.begin(), sot = source.end(); sit != sot; sit++) {
      string fn_full = path + "/" + folder[*sit] + "/" + fn;
      //cout << fn_full << endl;
      if (file_accessable(fn_full)) {
        const auto t0 = std::chrono::high_resolution_clock::now();

        // get tiles, add them
        int tile_size = 0;
        try {
          tile_size = 3600 / stoi(string(&sit->back())) + 1;
          // tile_size = 3600/int(sit->back()-'0') + 1; // also works ... chars are horrible ...
        }
        catch (const std::invalid_argument& ia) {
          cerr << "Invalid argument: " << ia.what() << endl;
        }
        catch (const std::out_of_range& oor) {
          std::cerr << "Out of Range error: " << oor.what() << endl;
        }
        // auto t1 = std::chrono::high_resolution_clock::now();
        // std::chrono::duration<double, std::milli> fp_ms = t1 - t0;
        // cout << "  preperation took " << fp_ms.count() << " ms" << endl;
        cout << "trying to read: " << fn_full << " with dimension " << tile_size << " ..." << endl; // flush;
        char const* FILENAME = fn_full.c_str();
        tile<int16_t> A(tile<int16_t>(FILENAME, tile_size, ref_lat, ref_lon));
        // auto t2 = std::chrono::high_resolution_clock::now();
        // fp_ms = t2 - t1;
        // cout << "  reading " << string(FILENAME) << " took " << fp_ms.count() << " ms" << endl;
        add_tile(A);
        // cout << " done" << endl;
        source_found = true;

        auto t3 = std::chrono::high_resolution_clock::now();
        // std::chrono::duration<double, std::milli>  fp_ms_2 = t3 - t2;
        std::chrono::duration<double, std::milli> fp_ms_tot = t3 - t0;
        // cout << "  adding tile took " << fp_ms_2.count() << " ms" << endl;
        cout << "  reading + processing tile " << string(FILENAME) << " took " << fp_ms_tot.count() << " ms" << endl;

        break; // add only one version of each tile
      }
    }
    if (!source_found)
      cerr << " no source for " + fn + " found, ignoring it" << endl;
  }
  if (z_standpoint == -1) {
    const double z_offset = 10; // asssume we are floating in midair to avoid artefacts
    // FIXME the case when points from neighbouring tiles are required
    // find the tile in which we are standing
    auto it = find_if(tiles.begin(), tiles.end(),
                      [&](const pair<tile<double>, tile<double>>& p) { return (p.first.get_lat() == floor(lat_standpoint * rad2deg)) && (p.first.get_lon() == floor(lon_standpoint * rad2deg)); });
    z_standpoint = (it->first).interpolate(lat_standpoint * rad2deg, lon_standpoint * rad2deg) + z_offset;
    cout << "overwriting the elevation: " << z_standpoint << endl;
  }
  cout << "scene constructed" << endl;
  debug.close();
}
