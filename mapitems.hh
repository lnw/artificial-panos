#pragma once

#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

#include "auxiliary.hh"
#include "latlon.hh"

#define NO_DEPR_DECL_WARNINGS_START _Pragma("GCC diagnostic push")
_Pragma("GCC diagnostic ignored \"-Wdeprecated-declarations\"")
#define NO_DEPR_DECL_WARNINGS_END _Pragma("GCC diagnostic pop")

    NO_DEPR_DECL_WARNINGS_START
#include <libxml++/libxml++.h>
    NO_DEPR_DECL_WARNINGS_END


    class scene;
class canvas;

struct point_feature {
  LatLon<double, Unit::deg> coords;
  std::string name;
  int elev; // provided by osm data, otherwise interpolated from elevation data

  point_feature(LatLon<double, Unit::deg> ll, std::string n, int e): coords(ll), name(std::move(n)), elev(e) {}
  point_feature(LatLon<double, Unit::deg> ll, int e): coords(ll), name(""), elev(e) {}
  point_feature(LatLon<double, Unit::deg> ll, std::string n): coords(ll), name(std::move(n)), elev() {}

  constexpr auto lat() const { return coords.lat(); }
  constexpr auto lon() const { return coords.lon(); }

  friend std::ostream& operator<<(std::ostream& S, const point_feature& pf) {
    S << "{(" << pf.coords.lat() << ", " << pf.coords.lon() << "), " << pf.name << ", " << pf.elev << "}";
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
  std::vector<LatLon<double, Unit::deg>> coords;
  std::string name;
  size_t id;   // so we can deduplicate between tiles
  bool closed; // detect by comparing first and last element

  linear_feature(): coords(), name(""), id(0), closed(false){};
  explicit linear_feature(int N): coords(N), name(""), id(0), closed(false){};

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
