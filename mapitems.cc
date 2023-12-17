
#include <climits>
#include <iomanip>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

#include "auxiliary.hh"
#include "canvas.hh"
#include "mapitems.hh"
#include "scene.hh"
#include "tile.hh"

#define NO_DEPR_DECL_WARNINGS_START _Pragma("GCC diagnostic push")
_Pragma("GCC diagnostic ignored \"-Wdeprecated-declarations\"")
#define NO_DEPR_DECL_WARNINGS_END _Pragma("GCC diagnostic pop")

NO_DEPR_DECL_WARNINGS_START
#include <libxml++/libxml++.h>
NO_DEPR_DECL_WARNINGS_END


template <typename T>
linear_feature_on_canvas<T>::linear_feature_on_canvas(const linear_feature<T>& _lf, const canvas<T>& C, const scene<T>& S): lf(_lf) {
  const T z_ref = S.z_standpoint;
  const T view_dir_h = S.view_dir_h;
  const T view_dir_v = S.view_dir_v;
  const T view_width = S.view_width;
  const T view_height = S.view_height;
  const T pixels_per_rad_h = C.xs() / view_width;
  const T pixels_per_rad_v = C.ys() / view_height; // [px/rad]

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
    const T x = std::fmod(view_dir_h + view_width / 2.0 + bearing(S.standpoint, point_r) + 1.5 * M_PI, 2 * M_PI) * pixels_per_rad_h;
    std::cout << " x: " << x << std::flush;
    const T y = (view_height / 2.0 + view_dir_v - angle_v(z_ref, z, dist)) * pixels_per_rad_v; // [px]
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
void parse_peaks_gpx(const xmlpp::Node* node, std::vector<point_feature<T>>& peaks) {
  const auto* nodeContent = dynamic_cast<const xmlpp::ContentNode*>(node);
  // const xmlpp::TextNode* nodeText = dynamic_cast<const xmlpp::TextNode*>(node);
  // const xmlpp::CommentNode* nodeComment = dynamic_cast<const xmlpp::CommentNode*>(node);

  if (node->get_name() == "node") { // the only interesting leaf
    const auto* nodeElement = dynamic_cast<const xmlpp::Element*>(node);
    // lat und lon are attributes of 'node'
    const LatLon<T, Unit::deg> ll{convert_from_stringish<T>(nodeElement->get_attribute("lat")->get_value()),
                                  convert_from_stringish<T>(nodeElement->get_attribute("lon")->get_value())};
    // std::cout << nodeElement->get_attribute("lat")->get_value() << std::endl;
    // std::cout << nodeElement->get_attribute("lon")->get_value() << std::endl;
    // std::cout << lat << ", " << lon << std::endl;
    T ele = 0;
    std::string name;
    for (const xmlpp::Node* child : node->get_children()) {
      if (child->get_name() == "tag") {
        const auto* child_el = dynamic_cast<const xmlpp::Element*>(child);
        // std::cout << "looking at childnodes" << std::endl;
        if (child_el->get_attribute("k")->get_value() == "ele") {
          // std::cout << "ele found" << std::endl;
          ele = convert_from_stringish<T>(child_el->get_attribute("v")->get_value());
          // std::cout << "ele: " << ele << std::endl;
        }
        if (child_el->get_attribute("k")->get_value() == "name") {
          // std::cout << "name found" << std::endl;
          name = child_el->get_attribute("v")->get_value();
          // std::cout << "name: " << name << std::endl;
        }
      }
    }
    peaks.emplace_back(ll, name, ele);
  }
  else if (nodeContent == nullptr) { // not a leaf
    // Recurse through child nodes:
    for (const xmlpp::Node* child : node->get_children()) {
      parse_peaks_gpx(child, peaks);
    }
  }
}


// parses the xml object, first gathers all coordinates with IDs, and all
// ways/realtions with lists of ID; then compiles vectors of points, ie linear
// features
template <typename T>
void gather_points(const xmlpp::Node* node, std::unordered_map<size_t, LatLon<T, Unit::deg>>& points) {
  const auto* nodeContent = dynamic_cast<const xmlpp::ContentNode*>(node);
  // const xmlpp::TextNode* nodeText = dynamic_cast<const xmlpp::TextNode*>(node);
  // const xmlpp::CommentNode* nodeComment = dynamic_cast<const xmlpp::CommentNode*>(node);

  if (node->get_name() == "node") {
    const auto* nodeElement = dynamic_cast<const xmlpp::Element*>(node);
    // id, lat, and lon are attributes of 'node'
    const size_t id = convert_from_stringish<size_t>(nodeElement->get_attribute("id")->get_value());
    const T lat = convert_from_stringish<T>(nodeElement->get_attribute("lat")->get_value());
    const T lon = convert_from_stringish<T>(nodeElement->get_attribute("lon")->get_value());
    // std::cout << id << ", " << lat << ", " << lon << std::endl;
    points.insert({id, LatLon<T, Unit::deg>(lat, lon)});
  }
  else if (nodeContent == nullptr) {
    // Recurse through child nodes:
    for (const xmlpp::Node* child : node->get_children()) {
      gather_points(child, points);
    }
  }
}

// parses the xml object, first gathers all coordinates with IDs, and all
// ways/realtions with lists of ID; then compiles vectors of points, ie linear
// features
void gather_ways(const xmlpp::Node* node, std::vector<std::pair<std::vector<size_t>, size_t>>& ways) {
  const auto* nodeContent = dynamic_cast<const xmlpp::ContentNode*>(node);
  //   const xmlpp::TextNode* nodeText = dynamic_cast<const xmlpp::TextNode*>(node);
  //   const xmlpp::CommentNode* nodeComment = dynamic_cast<const xmlpp::CommentNode*>(node);

  // 'way' contains a list of 'nd'-nodes
  if (node->get_name() == "way") {
    const auto* el = dynamic_cast<const xmlpp::Element*>(node);
    const size_t way_id = convert_from_stringish<size_t>(el->get_attribute("id")->get_value());
    std::vector<size_t> way_tmp;
    for (const xmlpp::Node* child : node->get_children()) {
      if (child->get_name() == "nd") {
        const auto* child_el = dynamic_cast<const xmlpp::Element*>(child);
        const size_t id = convert_from_stringish<size_t>(child_el->get_attribute("ref")->get_value());
        way_tmp.push_back(id);
        // std::cout << id << std::endl;
      }
    }
    // std::cout << way_tmp << std::endl;
    if (!way_tmp.empty()) {
      ways.emplace_back(way_tmp, way_id);
    }
  }
  else if (nodeContent == nullptr) {
    // Recurse through child nodes:
    for (const xmlpp::Node* child : node->get_children()) {
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
    xmlpp::DomParser parser;
    parser.parse_file(filename);
    if (parser) {
      // find root node
      const xmlpp::Node* pNode = parser.get_document()->get_root_node();
      // print recursively
      parse_peaks_gpx(pNode, peaks);
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

  std::unordered_map<size_t, LatLon<T, Unit::deg>> nodes;
  std::vector<std::pair<std::vector<size_t>, size_t>> ways;
  try {
    xmlpp::DomParser parser;
    parser.parse_file(filename);
    if (parser) {
      // find root node
      const xmlpp::Node* root = parser.get_document()->get_root_node();
      // collect all coordinates/IDs
      gather_points(root, nodes);
      std::cout << "found " << nodes.size() << " nodes" << std::endl;

      // collect ways
      gather_ways(root, ways);
      std::cout << "found " << ways.size() << " ways" << std::endl;
    }
  }
  catch (const std::exception& ex) {
    std::cout << "Exception caught: " << ex.what() << std::endl;
    abort();
  }

  // assemble ways/coordinates
  std::vector<linear_feature<T>> coastlines(ways.size());
  for (int64_t i = 0; i < std::size(ways); i++) {
    linear_feature<T> lf_tmp;
    for (int j = 0; j < static_cast<int>(ways[i].first.size()); j++) {
      const size_t id = ways[i].first[j];
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
    xmlpp::DomParser parser;
    parser.parse_file(filename);
    if (parser) {
      // find root node
      const xmlpp::Node* pNode = parser.get_document()->get_root_node();
      // print recursively
      //  parse_island_gpx(pNode, islands);
    }
  }
  catch (const std::exception& ex) {
    std::cout << "Exception caught: " << ex.what() << std::endl;
    abort();
  }

  std::cout << " done" << std::endl;
  return islands;
}
