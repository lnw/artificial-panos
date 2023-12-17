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

template <typename T>
class scene;
template <typename T>
class canvas;

template <typename T>
struct point_feature {
  LatLon<T, Unit::deg> coords;
  std::string name;
  int elev; // provided by osm data, otherwise interpolated from elevation data

  point_feature(LatLon<T, Unit::deg> ll, std::string n, int e): coords(ll), name(std::move(n)), elev(e) {}
  point_feature(LatLon<T, Unit::deg> ll, int e): coords(ll), elev(e) {}
  point_feature(LatLon<T, Unit::deg> ll, std::string n): coords(ll), name(std::move(n)), elev() {}

  constexpr auto lat() const { return coords.lat(); }
  constexpr auto lon() const { return coords.lon(); }

  friend std::ostream& operator<<(std::ostream& S, const point_feature& pf) {
    S << "{(" << pf.coords.lat() << ", " << pf.coords.lon() << "), " << pf.name << ", " << pf.elev << "}";
    return S;
  }
};

template <typename T>
struct point_feature_on_canvas {
  point_feature<T> pf;
  int x, y;
  T dist;
  int64_t xshift = 0;

  point_feature_on_canvas(point_feature<T> _pf, int _x, int _y, T _dist): pf(std::move(_pf)), x(_x), y(_y), dist(_dist) {}

  friend std::ostream& operator<<(std::ostream& S, const point_feature_on_canvas& pfc) {
    S << pfc.pf << " at (" << pfc.x << ", " << pfc.y << ", " << pfc.dist << ")";
    return S;
  }
};

template <typename T>
struct linear_feature {
  std::vector<LatLon<T, Unit::deg>> coords;
  std::string name;
  size_t id = 0;       // so we can deduplicate between tiles
  bool closed = false; // detect by comparing first and last element

  linear_feature() = default;
  explicit linear_feature(int N): coords(N){};

  bool operator<(const linear_feature& lf) const { return this->id < lf.id; };

  size_t size() const { return coords.size(); }
  void append(const LatLon<T, Unit::deg>& p) {
    coords.push_back(p);
  }

  friend std::ostream& operator<<(std::ostream& S, const linear_feature& lf) {
    S << "{" << lf.coords << ", " << lf.name << ", " << lf.closed << "}";
    return S;
  }
};

template <typename T>
struct linear_feature_on_canvas {
  linear_feature<T> lf;
  std::vector<T> xs, ys;
  std::vector<T> dists;

  linear_feature_on_canvas(const linear_feature<T>& _lf, const canvas<T>& C, const scene<T>& S);

  size_t size() const { return lf.size(); }

  bool operator<(const linear_feature_on_canvas& lfoc) const { return this->lf.id < lfoc.lf.id; };

  friend std::ostream& operator<<(std::ostream& S, const linear_feature_on_canvas& lfoc) {
    S << "[" << lfoc.lf << ", " << lfoc.dists << "]";
    return S;
  }
};

// parses the xml object, appends peaks
template <typename T>
void parse_peaks_gpx(const xmlpp::Node* node, std::vector<point_feature<T>>& peaks);

// parses the xml object, first gathers all coordinates with IDs, and all
// ways/realtions with lists of ID; then compiles vectors of points, ie linear
// features
template <typename T>
void parse_coast_gpx(const xmlpp::Node* node, std::vector<linear_feature<T>>& coasts);

// read plain xml
template <typename T>
std::vector<point_feature<T>> read_peaks_osm(const std::string& filename);

template <typename T>
std::vector<linear_feature<T>> read_coast_osm(const std::string& filename);

template <typename T>
std::vector<linear_feature<T>> read_islands_osm(const std::string& filename);
