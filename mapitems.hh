#pragma once

#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

#include "auxiliary.hh"

NO_DEPR_DECL_WARNINGS_START
#include <libxml++/libxml++.h>
NO_DEPR_DECL_WARNINGS_END


class scene;
class canvas;

struct point_feature {
  double lat;
  double lon;
  std::string name;
  int elev; // provided by osm data, otherwise interpolated from elevation data

  point_feature(double la, double lo, std::string n, int e): lat(la), lon(lo), name(std::move(n)), elev(e) {}
  point_feature(double la, double lo, int e): lat(la), lon(lo), name(""), elev(e) {}
  point_feature(double la, double lo, std::string n): lat(la), lon(lo), name(std::move(n)), elev() {}

  friend std::ostream& operator<<(std::ostream& S, const point_feature& pf) {
    S << "{(" << pf.lat << ", " << pf.lon << "), " << pf.name << ", " << pf.elev << "}";
    return S;
  }
};

struct point_feature_on_canvas {
  point_feature pf;
  int x, y, dist;
  int xshift;

  point_feature_on_canvas(point_feature _pf, int _x, int _y, int _dist): pf(std::move(_pf)), x(_x), y(_y), dist(_dist), xshift(0) {}

  friend std::ostream& operator<<(std::ostream& S, const point_feature_on_canvas& pfc) {
    S << pfc.pf << " at (" << pfc.x << ", " << pfc.y << ", " << pfc.dist << ")";
    return S;
  }
};

struct linear_feature {
  std::vector<std::pair<double, double>> coords; // lat, lon
  std::string name;
  size_t id;   // so we can deduplicate between tiles
  bool closed; // detect by comparing first and last element

  linear_feature(): coords(), name(""), id(0), closed(false){};
  explicit linear_feature(int N): coords(N), name(""), id(0), closed(false){};

  bool operator<(const linear_feature& lf) const { return this->id < lf.id; };

  size_t size() const { return coords.size(); }
  void append(const std::pair<double, double>& p) {
    coords.push_back(p);
  }

  friend std::ostream& operator<<(std::ostream& S, const linear_feature& lf) {
    S << "{" << lf.coords << ", " << lf.name << ", " << lf.closed << "}";
    return S;
  }
};

struct linear_feature_on_canvas {
  linear_feature lf;
  std::vector<double> xs, ys, dists;

  linear_feature_on_canvas(const linear_feature& _lf, const canvas& C, const scene& S);

  size_t size() const { return lf.size(); }

  bool operator<(const linear_feature_on_canvas& lfoc) const { return this->lf.id < lfoc.lf.id; };

  friend std::ostream& operator<<(std::ostream& S, const linear_feature_on_canvas& lfoc) {
    S << "[" << lfoc.lf << ", " << lfoc.dists << "]";
    return S;
  }
};

// parses the xml object, appends peaks
void parse_peaks_gpx(const xmlpp::Node* node, std::vector<point_feature>& peaks);

// parses the xml object, first gathers all coordinates with IDs, and all
// ways/realtions with lists of ID; then compiles vectors of points, ie linear
// features
void parse_coast_gpx(const xmlpp::Node* node, std::vector<linear_feature>& coasts);

// read plain xml
std::vector<point_feature> read_peaks_osm(const std::string& filename);

std::vector<linear_feature> read_coast_osm(const std::string& filename);

std::vector<linear_feature> read_islands_osm(const std::string& filename);
