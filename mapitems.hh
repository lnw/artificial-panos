#pragma once

#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

#include "auxiliary.hh"
#include "latlon.hh"

namespace xmlpp {
class Node;
}

class scene;
template <typename dist_t>
class canvas;

struct point_feature {
  LatLon<double, Unit::deg> coords;
  std::string name;
  int elev; // provided by osm data, otherwise interpolated from elevation data

  point_feature(LatLon<double, Unit::deg> ll, std::string n, int e): coords(ll), name(std::move(n)), elev(e) {}
  point_feature(LatLon<double, Unit::deg> ll, int e): coords(ll), elev(e) {}
  point_feature(LatLon<double, Unit::deg> ll, std::string n): coords(ll), name(std::move(n)), elev() {}

  constexpr auto lat() const { return coords.lat(); }
  constexpr auto lon() const { return coords.lon(); }

  friend std::ostream& operator<<(std::ostream& S, const point_feature& pf) {
    S << "{(" << pf.coords.lat() << ", " << pf.coords.lon() << "), " << pf.name << ", " << pf.elev << "}";
    return S;
  }
};

template <typename dist_t>
struct point_feature_on_canvas {
  point_feature pf;
  int x, y;
  dist_t dist;
  int xshift;

  point_feature_on_canvas(point_feature _pf, int _x, int _y, dist_t _dist): pf(std::move(_pf)), x(_x), y(_y), dist(_dist), xshift(0) {}

  friend std::ostream& operator<<(std::ostream& S, const point_feature_on_canvas& pfc) {
    S << pfc.pf << " at (" << pfc.x << ", " << pfc.y << ", " << pfc.dist << ")";
    return S;
  }
};

struct linear_feature {
  std::vector<LatLon<double, Unit::deg>> coords;
  std::string name;
  size_t id = 0;       // so we can deduplicate between tiles
  bool closed = false; // detect by comparing first and last element

  linear_feature(): coords(){};
  explicit linear_feature(int N): coords(N){};

  bool operator<(const linear_feature& lf) const { return this->id < lf.id; };

  size_t size() const { return coords.size(); }
  void append(const LatLon<double, Unit::deg>& p) {
    coords.push_back(p);
  }

  friend std::ostream& operator<<(std::ostream& S, const linear_feature& lf) {
    S << "{" << lf.coords << ", " << lf.name << ", " << lf.closed << "}";
    return S;
  }
};

template <typename dist_t>
struct linear_feature_on_canvas {
  linear_feature lf;
  std::vector<double> xs, ys;
  std::vector<dist_t> dists;

  linear_feature_on_canvas(const linear_feature& _lf, const canvas<dist_t>& C, const scene& S);

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
