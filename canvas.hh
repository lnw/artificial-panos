#pragma once

#include <cassert>
#include <cstdint>
#include <cstdio>
#include <iostream>
#include <limits>
#include <string>
#include <tuple>
#include <vector>

#include <gd.h>

#include "array2d.hh"

class scene;
struct point_feature;
struct point_feature_on_canvas;


int get_tile_index(const scene& S, const double lat, const double lon);


struct colour {
  constexpr colour() = default;
  constexpr colour(int r, int g, int b): r_(r), g_(g), b_(b) {}
  uint8_t r_ = 0, g_ = 0, b_ = 0;

  operator uint32_t() const {
    return 127 << 24 | r_ << 16 | g_ << 8 | b_;
  }
};

template <typename S, typename T>
class array_zb {
private:
  array2D<S> zbuffer_;
  array2D<T> arr2d_;

public:
  array_zb(int x, int y): zbuffer_(x, y, std::numeric_limits<int>::max()), arr2d_(x, y, 0) {}

  constexpr int width() const { return arr2d_.width(); }
  constexpr int height() const { return arr2d_.height(); }

  auto& zb() { return zbuffer_; }
  constexpr const auto& zb() const { return zbuffer_; }
  S zb(int x, int y) const { return zbuffer_(x, y); }
  constexpr S& zb(int x, int y) { return zbuffer_(x, y); }

  auto& a2d() { return arr2d_; }
  constexpr const auto& a2d() const { return arr2d_; }
  T a2d(int x, int y) const { return arr2d_(x, y); }
  constexpr T& a2d(int x, int y) { return arr2d_(x, y); }

  array_zb operator+(const array_zb& rh) const { return array_zb(*this) += rh; }
  array_zb& operator+=(const array_zb& rh) {
    const int width = arr2d_.n();
    const int height = arr2d_.m();
    for (int x = 0; x < width; x++) {
      for (int y = 0; y < height; y++) {
        if (rh.zb(x, y) < zbuffer_(x, y)) { // rh is closer
          arr2d_(x, y) = rh.a2d(x, y);
          zbuffer_(x, y) = rh.zb(x, y);
        }
      }
    }
    return *this;
  }
};


class canvas_t {
private:
  int width_, height_; // [pixels]
  array_zb<double, int32_t> buffered_canvas;

public:
  canvas_t(int x, int y): width_(x), height_(y), buffered_canvas(x, y) {}

  constexpr int width() const { return width_; }
  constexpr int height() const { return height_; }

  array2D<double>& get_zb() { return buffered_canvas.zb(); }
  constexpr const array2D<double>& get_zb() const { return buffered_canvas.zb(); }
  double get_zb(int x, int y) const { return buffered_canvas.zb(x, y); }
  constexpr double& get_zb(int x, int y) { return buffered_canvas.zb(x, y); }

  array2D<int32_t>& get_wc() { return buffered_canvas.a2d(); }
  constexpr const array2D<int32_t>& get_wc() const { return buffered_canvas.a2d(); }
  int32_t get_wc(int x, int y) const { return buffered_canvas.a2d(x, y); }
  constexpr int32_t& get_wc(int x, int y) { return buffered_canvas.a2d(x, y); }

  // for each column, walk from top to bottom and colour a pixel dark if it is
  // much closer than the previous one.  Works only because mountains are
  // rarely overhanging or floating in mid-air
  void highlight_edges();

  // just write the pixel taking into account the zbuffer
  void write_pixel_zb(const int x, const int y, const double z, const colour& col) {
    if (z < buffered_canvas.zb(x, y)) {
      buffered_canvas.zb(x, y) = z;
      buffered_canvas.a2d(x, y) = uint32_t(col);
    }
  }

  // just write the pixel
  void write_pixel(const int x, const int y, const colour& col) {
    buffered_canvas.a2d(x, y) = uint32_t(col);
  }

  // true if any pixel was drawn
  void draw_triangle(const double x1, const double y1,
                     const double x2, const double y2,
                     const double x3, const double y3,
                     const double z,
                     const colour& col);


  void render_scene(const scene& S);

  void render_test();

  void bucket_fill(const int r, const int g, const int b);
};


class canvas {
private:
  int width_;
  int height_;
  const array2D<double>& zbuffer;
  std::string filename;
  gdImagePtr img_ptr = nullptr;

public:
  canvas(std::string fn, const canvas_t& core): width_(core.width()), height_(core.height()), zbuffer(core.get_zb()), filename(std::move(fn)) {
    const array2D<int32_t>& wc(core.get_wc());
    // allocate mem
    img_ptr = gdImageCreateTrueColor(width_, height_);
    for (int x = 0; x < width_; x++)
      for (int y = 0; y < height_; y++)
        img_ptr->tpixels[y][x] = wc(x, y); // assuming TrueColor
  }
  canvas& operator=(const canvas&) = delete;
  canvas& operator=(canvas&&) = default;
  canvas(const canvas&) = delete;
  canvas(canvas&&) = default;

  ~canvas() {
    // actually the file is only opened here
    FILE* png_ptr = fopen(filename.c_str(), "wb");
    // write to disk
    gdImagePng(img_ptr, png_ptr);
    fclose(png_ptr);
    gdImageDestroy(img_ptr);
  }

  constexpr int width() const { return width_; }
  constexpr int height() const { return height_; }

  // just write the pixel
  void write_pixel(const int x, const int y, const colour& col) {
    img_ptr->tpixels[y][x] = int32_t(col); // assuming TrueColor
  }

  // read the zbuffer, return true if the pixel would be drawn
  constexpr bool would_write_pixel_zb(const int x, const int y, const double z) const {
    if (z > zbuffer(x, y))
      return false;
    else
      return true;
  }

  // true if any pixel was drawn
  bool would_draw_triangle(const double x1, const double y1,
                           const double x2, const double y2,
                           const double x3, const double y3,
                           const double z) const;

  bool draw_line(const double x1, const double y1,
                 const double x2, const double y2,
                 const double z,
                 const colour& col);
  bool would_draw_line(const double x1, const double y1,
                       const double x2, const double y2,
                       const double z) const;

  void draw_tick(int x_tick, int tick_length, const std::string& str1, const std::string& str2 = "");

  // always do N, E, S, W
  // every 10 deg (always)
  // every 5 deg if there are less than 45 deg
  // every deg if there are less than 10 deg
  void label_axis(const scene& S);

  void annotate_peaks(const scene& S);

  bool peak_is_visible_v1(const scene& S, const point_feature& peak, const double dist_peak, const int tile_index) const;
  bool peak_is_visible_v2(const scene& S, const point_feature& peak, const double dist_peak) const;

  // test if a peak is visible by attempting to draw a few triangles around it,
  // if the zbuffer admits any pixel to be drawn, the peak is visible
  std::tuple<std::vector<point_feature_on_canvas>, std::vector<point_feature_on_canvas>> get_visible_peaks(std::vector<point_feature>& peaks, const scene& S);

  std::vector<point_feature_on_canvas> draw_visible_peaks(const std::vector<point_feature_on_canvas>& peaks_vis);

  void draw_invisible_peaks(const std::vector<point_feature_on_canvas>& peaks_invis,
                            const colour& col);

  void annotate_islands(const scene& S);
  void draw_coast(const scene& S);
};
