#pragma once

#include <cassert>
#include <cstdint>
#include <cstdio>
#include <iostream>
#include <limits>
#include <memory>
#include <string>
#include <tuple>
#include <vector>

#include <gd.h>

#include "array2d.hh"
#include "colour.hh"

class scene;
struct point_feature;
struct point_feature_on_canvas;


int64_t get_tile_index(const scene& S, LatLon<double, Unit::deg> point);


template <typename S, typename T>
class array_zb {
public:
  array_zb(int64_t x, int64_t y): zbuffer_(x, y, std::numeric_limits<S>::max()), arr2d_(x, y, 0) {}

  constexpr int64_t xs() const { return arr2d_.xs(); }
  constexpr int64_t ys() const { return arr2d_.ys(); }

  constexpr auto& zb() & { return zbuffer_; }
  constexpr const auto& zb() const& { return zbuffer_; }
  constexpr auto&& zb() && { return zbuffer_; }
  constexpr S zb(int64_t x, int64_t y) const { return zbuffer_[x, y]; }
  constexpr S& zb(int64_t x, int64_t y) { return zbuffer_[x, y]; }

  constexpr auto& a2d() & { return arr2d_; }
  constexpr const auto& a2d() const& { return arr2d_; }
  constexpr auto&& a2d() && { return arr2d_; }
  constexpr T a2d(int64_t x, int64_t y) const { return arr2d_[x, y]; }
  constexpr T& a2d(int64_t x, int64_t y) { return arr2d_[x, y]; }

  array_zb operator+(const array_zb& rh) const { return array_zb(*this) += rh; }
  array_zb& operator+=(const array_zb& rh) {
    for (int64_t y = 0; y < ys(); y++) {
      for (int64_t x = 0; x < xs(); x++) {
        if (rh.zb(x, y) < zbuffer_[x, y]) { // rh is closer
          arr2d_[x, y] = rh.a2d(x, y);
          zbuffer_[x, y] = rh.zb(x, y);
        }
      }
    }
    return *this;
  }

private:
  array2D<S> zbuffer_;
  array2D<T> arr2d_;
};


class canvas_t {
public:
  canvas_t(int64_t xs, int64_t ys): xs_(xs), ys_(ys), buffered_canvas(xs, ys) {}

  constexpr int64_t xs() const { return xs_; }
  constexpr int64_t ys() const { return ys_; }

  // z buffer
  auto& zb() & { return buffered_canvas.zb(); }
  const auto& zb() const& { return buffered_canvas.zb(); }
  auto&& zb() && { return std::move(buffered_canvas).zb(); }
  double zb(int64_t x, int64_t y) const { return buffered_canvas.zb(x, y); }
  constexpr double& zb(int64_t x, int64_t y) { return buffered_canvas.zb(x, y); }

  // canvas
  constexpr auto& wc() & { return buffered_canvas.a2d(); }
  constexpr const auto& wc() const& { return buffered_canvas.a2d(); }
  constexpr auto&& wc() && { return std::move(buffered_canvas).a2d(); }
  constexpr int32_t wc(int64_t x, int64_t y) const { return buffered_canvas.a2d(x, y); }
  constexpr int32_t& wc(int64_t x, int64_t y) { return buffered_canvas.a2d(x, y); }

  // for each column, walk from top to bottom and colour a pixel dark if it is
  // much closer than the previous one.  Works only because mountains are
  // rarely overhanging or floating in mid-air
  void highlight_edges();

  // just write the pixel taking into account the zbuffer
  void write_pixel_zb(const int64_t x, const int64_t y, const double z, const colour& col) {
    if (z < buffered_canvas.zb(x, y)) {
      buffered_canvas.zb(x, y) = z;
      buffered_canvas.a2d(x, y) = int32_t(col);
    }
  }

  // just write the pixel
  void write_pixel(const int64_t x, const int64_t y, const colour& col) {
    buffered_canvas.a2d(x, y) = int32_t(col);
  }

  // true if any pixel was drawn
  void draw_triangle(double x1, double y1, double x2, double y2, double x3, double y3, double z,
                     const colour& col);

  void render_scene(const scene& S);

  void render_test();

  void bucket_fill(uint8_t r, uint8_t g, uint8_t b);

private:
  int64_t xs_, ys_; // [pixels]
  array_zb<double, int32_t> buffered_canvas;
};


class canvas {
  class gdDel_t {
  public:
    void operator()(gdImage* p) const {
      if (p)
        gdImageDestroy(p);
    }
  };

  class close_t {
  public:
    void operator()(FILE* p) const {
      if (p)
        fclose(p);
    }
  };

public:
  canvas(std::string fn, canvas_t core): xs_(core.xs()), ys_(core.ys()), zbuffer(std::move(core).zb()), filename(std::move(fn)), img_ptr(gdImageCreateTrueColor(xs_, ys_), gdDel_t{}) {
    const array2D<int32_t> wc(std::move(core).wc());
    // allocate mem
    for (int64_t y = 0; y < ys_; y++)
      for (int64_t x = 0; x < xs_; x++)
        img_ptr->tpixels[y][x] = wc[x, y]; // assuming TrueColor
  }

  void write_png() {
    std::unique_ptr<FILE, close_t> png_ptr(fopen(filename.c_str(), "wb"), close_t{});
    // write to disk
    gdImagePng(img_ptr.get(), png_ptr.get());
  }

  constexpr int64_t xs() const { return xs_; }
  constexpr int64_t ys() const { return ys_; }

  // just write the pixel
  void write_pixel(const int64_t x, const int64_t y, const colour& col) {
    img_ptr->tpixels[y][x] = int32_t(col); // assuming TrueColor
  }

  // read the zbuffer, return true if the pixel would be drawn
  constexpr bool would_write_pixel_zb(const int64_t x, const int64_t y, const double z) const {
    return z < zbuffer[x, y];
  }

  // true if any pixel was drawn
  bool would_draw_triangle(double x1, double y1,
                           double x2, double y2,
                           double x3, double y3,
                           double z) const;

  bool draw_line(double x1, double y1,
                 double x2, double y2,
                 double z,
                 const colour& col);
  bool would_draw_line(double x1, double y1,
                       double x2, double y2,
                       double z) const;

  void draw_tick(int x_tick, int tick_length, const std::string& str1, const std::string& str2 = "");

  // always do N, E, S, W
  // every 10 deg (always)
  // every 5 deg if there are less than 45 deg
  // every deg if there are less than 10 deg
  void label_axis(const scene& S);

  void annotate_peaks(const scene& S);

  bool peak_is_visible_v1(const scene& S, const point_feature& peak, double dist_peak, int64_t tile_index) const;
  bool peak_is_visible_v2(const scene& S, const point_feature& peak, double dist_peak) const;

  // test if a peak is visible by attempting to draw a few triangles around it,
  // if the zbuffer admits any pixel to be drawn, the peak is visible
  std::tuple<std::vector<point_feature_on_canvas>, std::vector<point_feature_on_canvas>> get_visible_peaks(std::vector<point_feature>& peaks, const scene& S);

  std::vector<point_feature_on_canvas> draw_visible_peaks(const std::vector<point_feature_on_canvas>& peaks_vis);

  void draw_invisible_peaks(const std::vector<point_feature_on_canvas>& peaks_invis,
                            const colour& col);

  void annotate_islands(const scene& S);
  void draw_coast(const scene& S);

private:
  int64_t xs_;
  int64_t ys_;
  array2D<double> zbuffer;
  std::string filename;
  std::unique_ptr<gdImage, gdDel_t> img_ptr; // which contains: int** tpixels
};
