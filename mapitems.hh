#ifndef MAPITEMS_HH
#define MAPITEMS_HH

#include <vector>
#include <iostream>
#include <string>
#include <iomanip> //required for setfill()
#include <libxml++/libxml++.h>

#include "auxiliary.hh"

using namespace std;


struct point_feature{
  double lat;
  double lon;
  string name;
  int elev;

  point_feature(double la, double lo, string n, int e): lat(la), lon(lo), name(n), elev(e) {}
  point_feature(double la, double lo, int e): lat(la), lon(lo), name(""), elev(e) {}
  point_feature(double la, double lo, string n): lat(la), lon(lo), name(n), elev() {}

  friend ostream& operator<<(ostream& S, const point_feature& pf)
  {
    S << "{(" << pf.lat << ", " << pf.lon << "), " << pf.name << ", " << pf.elev << "}";
    return S;
  }
};

struct linear_feature{
  vector<pair<double, double>> coords; // lat, lon
  string name;
  bool closed;

  friend ostream& operator<<(ostream& S, const linear_feature& lf)
  {
    S << "{" << lf.coords << ", " << lf.name << ", " << lf.closed << "}";
    return S;
  }
};


// parses te xml object, appends peaks
void parse_gpx(const xmlpp::Node *node, vector<point_feature> &peaks) {
  const xmlpp::ContentNode* nodeContent = dynamic_cast<const xmlpp::ContentNode*>(node);
  const xmlpp::TextNode* nodeText = dynamic_cast<const xmlpp::TextNode*>(node);
  const xmlpp::CommentNode* nodeComment = dynamic_cast<const xmlpp::CommentNode*>(node);

  Glib::ustring nodename = node->get_name();
  //cout << "name of node: " << nodename << endl;

  if(nodename.compare("node") == 0) {
    const xmlpp::Element* nodeElement = dynamic_cast<const xmlpp::Element*>(node);
    // lat und lon are attributes of 'node'
    const double lat = to_double(nodeElement->get_attribute("lat")->get_value());
    const double lon = to_double(nodeElement->get_attribute("lon")->get_value());
// cout << nodeElement->get_attribute("lat")->get_value() << endl;
// cout << nodeElement->get_attribute("lon")->get_value() << endl;
// cout << lat << ", " << lon << endl;
    double ele = 0;
    string name = "";
    xmlpp::Node::NodeList list = node->get_children();
    for(xmlpp::Node::NodeList::iterator iter = list.begin(); iter != list.end(); ++iter)
    {
      if( (*iter)->get_name() == "tag" ){
      const xmlpp::Element* child_el = dynamic_cast<const xmlpp::Element*>(*iter);
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
    xmlpp::Node::NodeList list = node->get_children();
    for(xmlpp::Node::NodeList::iterator iter = list.begin(); iter != list.end(); ++iter) {
      parse_gpx(*iter, peaks);
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
      parse_gpx(pNode, peaks);
    }
  }
  catch(const exception& ex) {
    cout << "Exception caught: " << ex.what() << endl;
    abort();
  }

  cout << " done" << endl;
  return peaks;
}

#endif

