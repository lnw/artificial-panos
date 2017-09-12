
#include <vector>
#include <iostream>
#include <string>
#include <iomanip> //required for setfill()
#include <unordered_map>

#include <libxml++/libxml++.h>

#include "auxiliary.hh"
#include "mapitems.hh"

using namespace std;

// parses the xml object, appends peaks
void parse_peaks_gpx(const xmlpp::Node *node, vector<point_feature> &peaks) {
  const xmlpp::ContentNode* nodeContent = dynamic_cast<const xmlpp::ContentNode*>(node);
  // const xmlpp::TextNode* nodeText = dynamic_cast<const xmlpp::TextNode*>(node);
  // const xmlpp::CommentNode* nodeComment = dynamic_cast<const xmlpp::CommentNode*>(node);

  if(node->get_name() == "node") {
    const xmlpp::Element* nodeElement = dynamic_cast<const xmlpp::Element*>(node);
    // lat und lon are attributes of 'node'
    const double lat = to_double(nodeElement->get_attribute("lat")->get_value());
    const double lon = to_double(nodeElement->get_attribute("lon")->get_value());
// cout << nodeElement->get_attribute("lat")->get_value() << endl;
// cout << nodeElement->get_attribute("lon")->get_value() << endl;
// cout << lat << ", " << lon << endl;
    double ele = 0;
    string name = "";
    for(const xmlpp::Node *child: node->get_children()){
      if(child->get_name() == "tag" ){
        const xmlpp::Element* child_el = dynamic_cast<const xmlpp::Element*>(child);
        // cout << "looking at childnodes" << endl;
        if(child_el->get_attribute("k")->get_value() == "ele") {
          // cout << "ele found" << endl;
          ele = to_double(child_el->get_attribute("v")->get_value());
          // cout << "ele: " << ele << endl;
        }
        if(child_el->get_attribute("k")->get_value() == "name") {
          // cout << "name found" << endl;
          name = child_el->get_attribute("v")->get_value();
          // cout << "name: " << name << endl;
        }
      }
    }
    peaks.push_back(point_feature(lat, lon, name, ele));
  }
  else if(!nodeContent){
    //Recurse through child nodes:
    for(const xmlpp::Node *child: node->get_children()){
      parse_peaks_gpx(child, peaks);
    }
  }
}


// parses the xml object, first gathers all coordinates with IDs, and all
// ways/realtions with lists of ID; then compiles vectors of points, ie linear
// features
void gather_points(const xmlpp::Node *node, unordered_map<size_t, pair<double,double>> &points) {
  const xmlpp::ContentNode* nodeContent = dynamic_cast<const xmlpp::ContentNode*>(node);
  // const xmlpp::TextNode* nodeText = dynamic_cast<const xmlpp::TextNode*>(node);
  // const xmlpp::CommentNode* nodeComment = dynamic_cast<const xmlpp::CommentNode*>(node);

  if(node->get_name() == "node") {
    const xmlpp::Element* nodeElement = dynamic_cast<const xmlpp::Element*>(node);
    // id, lat, and lon are attributes of 'node'
    const size_t id =  to_st(nodeElement->get_attribute("id")->get_value());
    const double lat = to_double(nodeElement->get_attribute("lat")->get_value());
    const double lon = to_double(nodeElement->get_attribute("lon")->get_value());
    // cout << id << ", " << lat << ", " << lon << endl;
    points.insert({id,make_pair(lat,lon)});
  }
  else if(!nodeContent){
    //Recurse through child nodes:
    for(const xmlpp::Node *child: node->get_children()) {
      gather_points(child, points);
    }
  }
}

// parses the xml object, first gathers all coordinates with IDs, and all
// ways/realtions with lists of ID; then compiles vectors of points, ie linear
// features
void gather_ways(const xmlpp::Node *node, vector<pair<vector<size_t>,size_t>> &ways) {
  const xmlpp::ContentNode* nodeContent = dynamic_cast<const xmlpp::ContentNode*>(node);
//   const xmlpp::TextNode* nodeText = dynamic_cast<const xmlpp::TextNode*>(node);
//   const xmlpp::CommentNode* nodeComment = dynamic_cast<const xmlpp::CommentNode*>(node);

  // 'way' contains a list of 'nd'-nodes
  if(node->get_name() == "way") {
    const xmlpp::Element* el = dynamic_cast<const xmlpp::Element*>(node);
    const size_t way_id = to_st(el->get_attribute("id")->get_value());
    vector<size_t> way_tmp;
    for(const xmlpp::Node *child: node->get_children()) {
      if( child->get_name() == "nd" ){
        const xmlpp::Element* child_el = dynamic_cast<const xmlpp::Element*>(child);
        const size_t id = to_st(child_el->get_attribute("ref")->get_value());
        way_tmp.push_back(id);
        // cout << id << endl;
      }
    }
    //cout << way_tmp << endl;
    if(way_tmp.size()!=0) ways.push_back(make_pair(way_tmp,way_id));
  }
  else if(!nodeContent){
    //Recurse through child nodes:
    for(const xmlpp::Node* child: node->get_children()) {
      gather_ways(child, ways);
    }
  }
}


// read plain xml
vector<point_feature> read_peaks_osm(string filename){
  cout << "attempting to parse: " << filename << " ..." << flush;
  vector<point_feature> peaks;

  try {
    xmlpp::DomParser parser;
    parser.parse_file(filename);
    if(parser) {
      //find root node
      const xmlpp::Node* pNode = parser.get_document()->get_root_node();
      //print recursively
      parse_peaks_gpx(pNode, peaks);
    }
  }
  catch(const exception& ex) {
    cout << "Exception caught: " << ex.what() << endl;
    abort();
  }
  cout << " done" << endl;
  return peaks;
}


vector<linear_feature> read_coast_osm(string filename){
  cout << "attempting to parse: " << filename << " ..." << flush;

  unordered_map<size_t, pair<double, double>> nodes;
  vector<pair<vector<size_t>,size_t>> ways;
  try {
    xmlpp::DomParser parser;
    parser.parse_file(filename);
    if(parser) {
      //find root node
      const xmlpp::Node* root = parser.get_document()->get_root_node();
      // collect all coordinates/IDs
      gather_points(root, nodes);
      cout << "found " << nodes.size() << " nodes" << endl;

      // collect ways
      gather_ways(root, ways);
      cout << "found " << ways.size() << " ways" << endl;
    }
  }
  catch(const exception& ex) {
    cout << "Exception caught: " << ex.what() << endl;
    abort();
  }

  // assemble ways/coordinates
  vector<linear_feature> coastlines(ways.size());
  for(int i=0; i<ways.size(); i++){
    linear_feature lf_tmp;
    for(int j=0; j<ways[i].first.size(); j++){
      const size_t id = ways[i].first[j];
      const pair<double,double> coord(nodes[id]);
      lf_tmp.append(coord);
    }
    lf_tmp.id = ways[i].second;
    lf_tmp.closed = ways[i].first.front() == ways[i].first.back(); // are the first and the last id identical?
    if(lf_tmp.size()!=0) coastlines[i] = lf_tmp;
  }
  cout << " done" << endl;
  return coastlines;
}


vector<linear_feature> read_islands_osm(string filename){
   cout << "attempting to parse: " << filename << " ..." << flush;
   vector<linear_feature> islands;
// 
//   try {
//     xmlpp::DomParser parser;
//     parser.parse_file(filename);
//     if(parser) {
//       //find root node
//       const xmlpp::Node* pNode = parser.get_document()->get_root_node();
//       //print recursively
//       //parse_island_gpx(pNode, islands);
//     }
//   }
//   catch(const exception& ex) {
//     cout << "Exception caught: " << ex.what() << endl;
//     abort();
//   }
// 
//   cout << " done" << endl;
   return islands;
}


