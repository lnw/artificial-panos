#ifndef MAPITEMS_HH
#define MAPITEMS_HH

#include <vector>
#include <iostream>
#include <string>
#include <iomanip> //required for setfill()

#include <libxml++/libxml++.h>

#include "auxiliary.hh"
#include "geometry.hh"

using namespace std;


struct point_feature{
  double lat;
  double lon;
  string name;
  int elev; // provided by osm data, otherwise interpolated from elevation data

  point_feature(double la, double lo, string n, int e): lat(la), lon(lo), name(n), elev(e) {}
  point_feature(double la, double lo, int e): lat(la), lon(lo), name(""), elev(e) {}
  point_feature(double la, double lo, string n): lat(la), lon(lo), name(n), elev() {}

  friend ostream& operator<<(ostream& S, const point_feature& pf) {
    S << "{(" << pf.lat << ", " << pf.lon << "), " << pf.name << ", " << pf.elev << "}";
    return S;
  }
};

struct point_feature_on_canvas{
  point_feature pf;
  int x, y, dist;
  int xshift;

  point_feature_on_canvas(point_feature _pf, int _x, int _y, int _dist): pf(_pf), x(_x), y(_y), dist(_dist), xshift(0) {}

  friend ostream& operator<<(ostream& S, const point_feature_on_canvas& pfc) {
    S << pfc.pf << " at (" << pfc.x << ", " << pfc.y << ", " << pfc.dist << ")";
    return S;
  }
};

struct linear_feature{
  vector<pair<double, double>> coords; // lat, lon
  string name;
  size_t id; // so we can deduplicate between tiles
  bool closed; // detect by comparing first and last element

  linear_feature(): coords(), name(""), id(0), closed(false) {};
  linear_feature(int N): coords(N), name(""), id(0), closed(false) {};

  bool operator<(const linear_feature &lf) const {return id < lf.id;};

  size_t size() const {return coords.size();}
  void append(const pair<double,double> &p){
    coords.push_back(p);
  }

  friend ostream& operator<<(ostream& S, const linear_feature& lf) {
    S << "{" << lf.coords << ", " << lf.name << ", " << lf.closed << "}";
    return S;
  }
};

struct linear_feature_on_canvas{
  linear_feature lf;
  vector<double> dists;

  linear_feature_on_canvas(const linear_feature& _lf, double lat, double lon): lf(_lf){
    for(pair<double, double> point: lf.coords){
      const double dist = distance_atan(lat, lon, point.first, point.second);
      dists.push_back(dist);
    }
  };

  friend ostream& operator<<(ostream& S, const linear_feature_on_canvas& lfoc) {
    S << "[" << lfoc.lf << ", " << lfoc.dists << "]";
    return S;
  }
};

// parses the xml object, appends peaks
void parse_peaks_gpx(const xmlpp::Node *node, vector<point_feature> &peaks);

// parses the xml object, first gathers all coordinates with IDs, and all
// ways/realtions with lists of ID; then compiles vectors of points, ie linear
// features
void parse_coast_gpx(const xmlpp::Node *node, vector<linear_feature> &coasts);

// read plain xml
vector<point_feature> read_peaks_osm(string filename);

vector<linear_feature> read_coast_osm(string filename);

vector<linear_feature> read_islands_osm(string filename);

#endif

