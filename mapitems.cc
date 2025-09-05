#include "mapitems.hh"
#include "auxiliary.hh"
#include "canvas.hh"
#include "scene.hh"
#include "tile.hh"
#include <iomanip>
#include <iostream>
#include <limits>
#include <string>
#include <unordered_map>
#include <vector>

#include <tinyxml2.h>

template <typename T>
linear_feature_on_canvas<T>::linear_feature_on_canvas(const linear_feature<T>& _lf, const canvas<T>& C, const scene<T>& S): lf(_lf) {
  const T z_ref = S.z_standpoint_m;
  const T view_dir_h = S.view_dir_h;
  const T view_dir_v = S.view_dir_v;
  const T view_width = S.view_width;
  const T view_height = S.view_height;
  const T pixels_per_rad_h = C.xs() / view_width;
  const T pixels_per_rad_v = C.ys() / view_height; // [px/rad]
  const T pi = std::numbers::pi_v<T>;

  // iterate over points in linear feature
  for (const auto& point_d : lf.coords) {
    const auto point_r = point_d.to_rad();
    const int64_t tile_index = get_tile_index<T>(S, point_d);
    if (tile_index < 0) {
      xs.push_back(-1);
      ys.push_back(-1);
      dists.push_back(std::numeric_limits<T>::max());
      std::cout << "nope" << std::endl;
      continue;
    }
    const tile<T>& H = S.tiles[tile_index].first;
    std::cout << "H " << std::flush;
    const T z = H.interpolate(point_d);
    std::cout << " z: " << z << std::flush;
    // get position on canvas, continue if outside
    // std::cout << "lat/lon: " << lat_ref<<", "<< lon_ref<<", "<< lat_r << ", " << lon_r << std::endl;
    const T dist = distance_atan<T>(S.standpoint, point_r);
    std::cout << " dist: " << dist << std::flush;
    const T x = std::fmod(view_dir_h + view_width / 2 + bearing(S.standpoint, point_r) + T(1.5) * pi, 2 * pi) * pixels_per_rad_h;
    std::cout << " x: " << x << std::flush;
    const T y = (view_height / 2 + view_dir_v - angle_v(z_ref, z, dist)) * pixels_per_rad_v; // [px]
    std::cout << " y: " << y << std::flush;
    // std::cout << "peak x, y " << x_peak << ", " << y_peak << std::endl;
    // if(x < 0 || x > C.xs ) continue;
    // if(y < 0 || y > C.ys ) continue;
    xs.push_back(x);
    ys.push_back(y);
    dists.push_back(dist);
    std::cout << "end" << std::endl;
  }
}
template linear_feature_on_canvas<float>::linear_feature_on_canvas(const linear_feature<float>& _lf, const canvas<float>& C, const scene<float>& S);
template linear_feature_on_canvas<double>::linear_feature_on_canvas(const linear_feature<double>& _lf, const canvas<double>& C, const scene<double>& S);

// parses the xml object, appends peaks
template <typename T>
void parse_peaks_gpx(const tinyxml2::XMLElement* node, std::vector<point_feature<T>>& peaks) {

  if (std::strcmp(node->Name(), "node") == 0) { // the only leaf interesting to us
    // lat und lon are attributes of 'node'
    const T lat = node->FloatAttribute("lat");
    const T lon = node->FloatAttribute("lon");
    // std::cout << lat << ", " << lon << std::endl;
    T ele = 0;
    std::string name;
    for (const auto* child = node->FirstChildElement(); child != 0; child = child->NextSiblingElement()) {
      if (std::strcmp(child->Name(), "tag") == 0) {
        if (std::strcmp(child->Attribute("k"), "ele") == 0) {
          // std::cout << "ele found" << std::endl;
          ele = child->FloatAttribute("v");
          // std::cout << "ele: " << ele << std::endl;
        }
        if (std::strcmp(child->Attribute("k"), "name") == 0) {
          // std::cout << "name found" << std::endl;
          name = child->Attribute("v");
          // std::cout << "name: " << name << std::endl;
        }
      }
    }
    peaks.emplace_back(LatLon<T, Unit::deg>(lat, lon), name, ele);
  }
  else if (!node->NoChildren()) { // not a leaf
    // Recurse through child nodes:
    for (const auto* child = node->FirstChildElement(); child != 0; child = child->NextSiblingElement()) {
      parse_peaks_gpx(child, peaks);
    }
  }
}


// parses the xml object, first gathers all coordinates with IDs, and all
// ways/realtions with lists of ID; then compiles vectors of points, ie linear
// features
template <typename T>
void gather_points(const tinyxml2::XMLElement* node, std::unordered_map<uint64_t, LatLon<T, Unit::deg>>& points) {

  if (std::strcmp(node->Name(), "node") == 0) {
    // id, lat, and lon are attributes of 'node'
    const uint64_t id = node->Unsigned64Attribute("id");
    const T lat = node->FloatAttribute("lat");
    const T lon = node->FloatAttribute("lon");
    // std::cout << id << ", " << lat << ", " << lon << std::endl;
    points.insert({id, LatLon<T, Unit::deg>(lat, lon)});
  }
  else if (!node->NoChildren()) { // not a leaf
    // Recurse through child nodes:
    for (const auto* child = node->FirstChildElement(); child != 0; child = child->NextSiblingElement()) {
      gather_points(child, points);
    }
  }
}

// parses the xml object, first gathers all coordinates with IDs, and all
// ways/realtions with lists of ID; then compiles vectors of points, ie linear
// features
void gather_ways(const tinyxml2::XMLElement* node, std::vector<std::pair<std::vector<uint64_t>, uint64_t>>& ways) {
  // 'way' contains a list of 'nd'-nodes
  if (std::strcmp(node->Name(), "way") == 0) {
    const uint64_t way_id = node->Unsigned64Attribute("id");
    std::vector<uint64_t> way_tmp;
    for (const auto* child = node->FirstChildElement(); child != 0; child = child->NextSiblingElement()) {
      if (std::strcmp(child->Name(), "nd") == 0) {
        const uint64_t id = child->Unsigned64Attribute("ref");
        way_tmp.push_back(id);
        // std::cout << id << std::endl;
      }
    }
    // std::cout << way_tmp << std::endl;
    if (!way_tmp.empty()) {
      ways.emplace_back(way_tmp, way_id);
    }
  }
  else if (!node->NoChildren()) { // not a leaf
    // Recurse through child nodes:
    for (const auto* child = node->FirstChildElement(); child != 0; child = child->NextSiblingElement()) {
      gather_ways(child, ways);
    }
  }
}


// read plain xml
template <typename T>
std::vector<point_feature<T>> read_peaks_osm(const std::string& filename) {
  std::cout << "attempting to parse: " << filename << " ..." << std::flush;
  std::vector<point_feature<T>> peaks;

  try {
    tinyxml2::XMLDocument xml;
    xml.LoadFile(filename.c_str());

    if (!xml.Error()) {
      const tinyxml2::XMLElement* rootNode = xml.RootElement();
      // works recursively
      parse_peaks_gpx(rootNode, peaks);
    }
  }
  catch (const std::exception& ex) {
    std::cout << "Exception caught: " << ex.what() << std::endl;
    abort();
  }
  std::cout << " done" << std::endl;
  return peaks;
}
template std::vector<point_feature<float>> read_peaks_osm(const std::string& filename);
template std::vector<point_feature<double>> read_peaks_osm(const std::string& filename);


template <typename T>
std::vector<linear_feature<T>> read_coast_osm(const std::string& filename) {
  std::cout << "attempting to parse: " << filename << " ..." << std::flush;

  std::unordered_map<uint64_t, LatLon<T, Unit::deg>> nodes;
  std::vector<std::pair<std::vector<uint64_t>, uint64_t>> ways;
  try {
    tinyxml2::XMLDocument xml;
    xml.LoadFile(filename.c_str());

    if (!xml.Error()) {
      const tinyxml2::XMLElement* rootNode = xml.RootElement();
      // collect all coordinates/IDs
      gather_points(rootNode, nodes);
      std::cout << "found " << nodes.size() << " nodes" << std::endl;

      // collect ways
      gather_ways(rootNode, ways);
      std::cout << "found " << ways.size() << " ways" << std::endl;
    }
  }
  catch (const std::exception& ex) {
    std::cout << "Exception caught: " << ex.what() << std::endl;
    abort();
  }

  // assemble ways/coordinates
  std::vector<linear_feature<T>> coastlines(ways.size());
  for (int64_t i = 0; i < std::ssize(ways); i++) {
    linear_feature<T> lf_tmp;
    for (int64_t j = 0; j < std::ssize(ways[i].first); j++) {
      const uint64_t id = ways[i].first[j];
      lf_tmp.append(nodes[id]);
    }
    lf_tmp.id = ways[i].second;
    lf_tmp.closed = ways[i].first.front() == ways[i].first.back(); // are the first and the last id identical?
    if (lf_tmp.size() != 0)
      coastlines[i] = lf_tmp;
  }
  std::cout << " done" << std::endl;
  return coastlines;
}
template std::vector<linear_feature<float>> read_coast_osm(const std::string& filename);
template std::vector<linear_feature<double>> read_coast_osm(const std::string& filename);


template <typename T>
std::vector<linear_feature<T>> read_islands_osm(const std::string& filename) {
  std::cout << "attempting to parse: " << filename << " ..." << std::flush;
  std::vector<linear_feature<T>> islands;

  try {
    tinyxml2::XMLDocument xml;
    xml.LoadFile(filename.c_str());
    if (!xml.Error()) {
      // find root node
      const tinyxml2::XMLElement* rootNode = xml.RootElement();
      // print recursively
      //  parse_island_gpx(rootNode, islands);
    }
  }
  catch (const std::exception& ex) {
    std::cout << "Exception caught: " << ex.what() << std::endl;
    abort();
  }

  std::cout << " done" << std::endl;
  return islands;
}
