#ifndef CANVAS_HH
#define CANVAS_HH

#include <cmath>
#include <iostream>
#include <limits>
#include <tuple>
#include <vector>

#include <gd.h>

#include "array2D.hh"
#include "geometry.hh"
#include "mapitems.hh"
#include "scene.hh"
#include "tile.hh"

using namespace std;

enum class write_target { core,
                          imgptr };


int get_tile_index(const scene& S, const double lat, const double lon);


class canvas_t {
private:
  unsigned width, height; // [pixels]
  array2D<double> zbuffer;
  array2D<int32_t> working_canvas;

public:
  canvas_t(int x, int y): width(x), height(y), zbuffer(x, y, INT_MAX), working_canvas(x, y, 0) {}
  canvas_t(const canvas_t& c): width(c.get_width()), height(c.get_height()), zbuffer(c.get_zb()), working_canvas(c.get_wc()) {}

  canvas_t operator+(const canvas_t& rh) const { return canvas_t(*this) += rh; }
  canvas_t& operator+=(const canvas_t& rh) {
    for (int x = 0; x < width; x++) {
      for (int y = 0; y < height; y++) {
        if (rh.get_zb(x, y) < zbuffer(x, y))
          working_canvas(x, y) = rh.get_wc(x, y);
      }
    }
    return *this;
  }

  unsigned get_width() const { return width; }
  unsigned get_height() const { return height; }

  array2D<double> get_zb() const { return zbuffer; }
  const array2D<double>& get_zb() { return zbuffer; }
  double get_zb(int x, int y) const { return zbuffer(x, y); }
  double& get_zb(int x, int y) { return zbuffer(x, y); }

  array2D<int32_t> get_wc() const { return working_canvas; }
  const array2D<int32_t>& get_wc() { return working_canvas; }
  int32_t get_wc(int x, int y) const { return working_canvas(x, y); }
  int32_t& get_wc(int x, int y) { return working_canvas(x, y); }
};


class canvas {
private:
  canvas_t core;
  string filename;
  gdImagePtr img_ptr = nullptr;
  bool image_constructed;

public:
  canvas(string fn, int x, int y): core(x, y), filename(fn), image_constructed(false) {
  }

  ~canvas() {
    // actually the file is only opened here
    FILE* png_ptr = fopen(filename.c_str(), "wb");
    // write to disk
    gdImagePng(img_ptr, png_ptr);
    fclose(png_ptr);
    gdImageDestroy(img_ptr);
  }

  unsigned get_width() const { return core.get_width(); }
  unsigned get_height() const { return core.get_height(); }

  void construct_image() {
    assert(!image_constructed);
    const unsigned width(core.get_width()),
        height(core.get_height());
    const array2D<int32_t>& wc(core.get_wc());
    // allocate mem
    img_ptr = gdImageCreateTrueColor(width, height);
    for (size_t x = 0; x < width; x++)
      for (size_t y = 0; y < height; y++)
        img_ptr->tpixels[y][x] = wc(x, y); // assuming TrueColor
    image_constructed = true;
  }


  // just write the pixel
  template <write_target wt>
  void write_pixel(const int x, const int y,
                   int16_t r, int16_t g, int16_t b) {
    const int32_t col = 127 << 24 | r << 16 | g << 8 | b;
    if (wt == write_target::core)
      core.get_wc(x, y) = col;
    else if (wt == write_target::imgptr)
      img_ptr->tpixels[y][x] = col; // assuming TrueColor
  }


  // just write the pixel taking into account the zbuffer
  void write_pixel_zb(const int x, const int y, const double z,
                      int16_t r, int16_t g, int16_t b) {
    assert(!image_constructed);
    if (z < core.get_zb(x, y)) {
      core.get_zb(x, y) = z;
      const int32_t col = 127 << 24 | r << 16 | g << 8 | b;
      // img_ptr->tpixels[y][x] = col; // assuming TrueColor
      core.get_wc(x, y) = col;
    }
  }

  // read the zbuffer, return true if the pixel would be drawn
  bool would_write_pixel_zb(const int x, const int y, const double z) {
    if (z > core.get_zb(x, y))
      return false;
    else
      return true;
  }

  // true if any pixel was drawn
  void draw_triangle(const double x1, const double y1,
                     const double x2, const double y2,
                     const double x3, const double y3,
                     const double z,
                     int16_t r, int16_t g, int16_t b);

  // true if any pixel was drawn
  bool would_draw_triangle(const double x1, const double y1,
                           const double x2, const double y2,
                           const double x3, const double y3,
                           const double z);

  bool draw_line(const double x1, const double y1,
                 const double x2, const double y2,
                 const double z,
                 int16_t r, int16_t g, int16_t b,
                 bool draw);

  void draw_tick(int x_tick, int tick_length, string str1, string str2 = "");

  // always do N, E, S, W
  // every 10 deg (always)
  // every 5 deg if there are less than 45 deg
  // every deg if there are less than 10 deg
  void label_axis(const scene& S);

  void render_scene(const scene& S);

  // for each column, walk from top to bottom and colour a pixel dark if it is
  // much closer than the previous one.  Works only because mountains are
  // rarely overhanging or floating in mid-air
  void highlight_edges();

  void render_test();

  void bucket_fill(const int r, const int g, const int b);

  void annotate_peaks(const scene& S);

  bool peak_is_visible_v1(const scene& S, const point_feature peak, const double dist_peak, const int tile_index);
  bool peak_is_visible_v2(const scene& S, const point_feature peak, const double dist_peak);

  // test if a peak is visible by attempting to draw a few triangles around it,
  // if the zbuffer admits any pixel to be drawn, the peak is visible
  tuple<vector<point_feature_on_canvas>, vector<point_feature_on_canvas>> get_visible_peaks(vector<point_feature>& peaks, const scene& S);

  vector<point_feature_on_canvas> draw_visible_peaks(const vector<point_feature_on_canvas>& peaks_vis);

  void draw_invisible_peaks(const vector<point_feature_on_canvas>& peaks_invis,
                            const int16_t r, const int16_t g, const int16_t b);

  void annotate_islands(const scene& S);
  void draw_coast(const scene& S);
};

#endif
