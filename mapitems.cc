
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


template <typename dist_t>
linear_feature_on_canvas<dist_t>::linear_feature_on_canvas(const linear_feature& _lf, const canvas<dist_t>& C, const scene& S): lf(_lf) {
  const dist_t z_ref = S.z_standpoint;
  const double view_dir_h = S.view_dir_h;
  const double view_dir_v = S.view_dir_v;
  const double view_width = S.view_width;
  const double view_height = S.view_height;
  const double pixels_per_rad_h = C.xs() / view_width;
  const double pixels_per_rad_v = C.ys() / view_height; // [px/rad]

  // iterate over points in linear feature
  for (const auto& point_d : lf.coords) {
    const auto point_r = point_d.to_rad();
    const int tile_index = get_tile_index(S, point_d);
    if (tile_index < 0) {
      xs.push_back(-1);
      ys.push_back(-1);
      dists.push_back(std::numeric_limits<int>::max());
      std::cout << "nope" << std::endl;
      continue;
    }
    const auto& H = S.tiles[tile_index].first;
    std::cout << "H " << std::flush;
    const dist_t z = H.interpolate(point_d);
    std::cout << " z: " << z << std::flush;
    // get position on canvas, continue if outside
    // std::cout << "lat/lon: " << lat_ref<<", "<< lon_ref<<", "<< lat_r << ", " << lon_r << std::endl;
    const dist_t dist = distance_atan<dist_t>(S.standpoint, point_r);
    std::cout << " dist: " << dist << std::flush;
    const double x = fmod(view_dir_h + view_width / 2.0 + bearing(S.standpoint, point_r) + 1.5 * M_PI, 2 * M_PI) * pixels_per_rad_h;
    std::cout << " x: " << x << std::flush;
    const double y = (view_height / 2.0 + view_dir_v - angle_v(z_ref, z, dist)) * pixels_per_rad_v; // [px]
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
template linear_feature_on_canvas<float>::linear_feature_on_canvas(const linear_feature& _lf, const canvas<float>& C, const scene& S);
template linear_feature_on_canvas<double>::linear_feature_on_canvas(const linear_feature& _lf, const canvas<double>& C, const scene& S);

// parses the xml object, appends peaks
void parse_peaks_gpx(const xmlpp::Node* node, std::vector<point_feature>& peaks) {
  const auto* nodeContent = dynamic_cast<const xmlpp::ContentNode*>(node);
  // const xmlpp::TextNode* nodeText = dynamic_cast<const xmlpp::TextNode*>(node);
  // const xmlpp::CommentNode* nodeComment = dynamic_cast<const xmlpp::CommentNode*>(node);

  if (node->get_name() == "node") { // the only interesting leaf
    const auto* nodeElement = dynamic_cast<const xmlpp::Element*>(node);
    // lat und lon are attributes of 'node'
    const LatLon<double, Unit::deg> ll{convert_from_stringish<double>(nodeElement->get_attribute("lat")->get_value()),
                                       convert_from_stringish<double>(nodeElement->get_attribute("lon")->get_value())};
    // std::cout << nodeElement->get_attribute("lat")->get_value() << std::endl;
    // std::cout << nodeElement->get_attribute("lon")->get_value() << std::endl;
    // std::cout << lat << ", " << lon << std::endl;
    double ele = 0;
    std::string name;
    for (const xmlpp::Node* child : node->get_children()) {
      if (child->get_name() == "tag") {
        const auto* child_el = dynamic_cast<const xmlpp::Element*>(child);
        // std::cout << "looking at childnodes" << std::endl;
        if (child_el->get_attribute("k")->get_value() == "ele") {
          // std::cout << "ele found" << std::endl;
          ele = convert_from_stringish<double>(child_el->get_attribute("v")->get_value());
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
void gather_points(const xmlpp::Node* node, std::unordered_map<size_t, LatLon<double, Unit::deg>>& points) {
  const auto* nodeContent = dynamic_cast<const xmlpp::ContentNode*>(node);
  // const xmlpp::TextNode* nodeText = dynamic_cast<const xmlpp::TextNode*>(node);
  // const xmlpp::CommentNode* nodeComment = dynamic_cast<const xmlpp::CommentNode*>(node);

  if (node->get_name() == "node") {
    const auto* nodeElement = dynamic_cast<const xmlpp::Element*>(node);
    // id, lat, and lon are attributes of 'node'
    const size_t id = convert_from_stringish<size_t>(nodeElement->get_attribute("id")->get_value());
    const double lat = convert_from_stringish<double>(nodeElement->get_attribute("lat")->get_value());
    const double lon = convert_from_stringish<double>(nodeElement->get_attribute("lon")->get_value());
    // std::cout << id << ", " << lat << ", " << lon << std::endl;
    points.insert({id, LatLon<double, Unit::deg>(lat, lon)});
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
std::vector<point_feature> read_peaks_osm(const std::string& filename) {
  std::cout << "attempting to parse: " << filename << " ..." << std::flush;
  std::vector<point_feature> peaks;

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


std::vector<linear_feature> read_coast_osm(const std::string& filename) {
  std::cout << "attempting to parse: " << filename << " ..." << std::flush;

  std::unordered_map<size_t, LatLon<double, Unit::deg>> nodes;
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
  std::vector<linear_feature> coastlines(ways.size());
  for (int i = 0; i < static_cast<int>(ways.size()); i++) {
    linear_feature lf_tmp;
    for (int j = 0; j < static_cast<int>(ways[i].first.size()); j++) {
      const size_t id = ways[i].first[j];
      // const std::pair<double, double> coord(nodes[id]);
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


std::vector<linear_feature> read_islands_osm(const std::string& filename) {
  std::cout << "attempting to parse: " << filename << " ..." << std::flush;
  std::vector<linear_feature> islands;

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
