#ifndef CANVAS_HH
#define CANVAS_HH

#include <cmath>
#include <iostream>
#include <tuple>
#include <vector>
#include <limits.h>

#include <gd.h>

#include "geometry.hh"
#include "array2D.hh"
#include "tile.hh"
#include "scene.hh"
#include "mapitems.hh"

using namespace std;

int get_tile_index(const scene& S, const double lat, const double lon);

class canvas {
public:
  unsigned width, height; // [pixels]

private:
  array2D<double> zbuffer;
  string filename;
  gdImagePtr img_ptr = nullptr;

public:
  canvas(string fn, int x, int y): width(x), height(y), zbuffer(x,y,INT_MAX), filename(fn){
    // allocate mem
    img_ptr = gdImageCreateTrueColor(width, height);
  }

  ~canvas(){
    // actually the file is only opened here
    FILE *png_ptr = fopen(filename.c_str(), "wb");
    // write to disk
    gdImagePng(img_ptr, png_ptr);
    fclose(png_ptr);
    gdImageDestroy(img_ptr);
  }

  // just write the pixel
  void write_pixel(const int x, const int y,
                   int16_t r, int16_t g, int16_t b){
    const int32_t col = 127 << 24 | r << 16 | g << 8 | b ;
    img_ptr->tpixels[y][x] = col; // assuming TrueColor
  }

  // just write the pixel taking into account the zbuffer
  // true if pixel was drawn
  void write_pixel_zb(const int x, const int y, const double z,
                      int16_t r, int16_t g, int16_t b){
    if (z < zbuffer(x,y)){
      zbuffer(x,y) = z;
      const int32_t col = 127 << 24 | r << 16 | g << 8 | b ;
      img_ptr->tpixels[y][x] = col; // assuming TrueColor
    }
  }

  // just write the pixel taking into account the zbuffer
  // true if pixel was drawn
  bool would_write_pixel_zb(const int x, const int y, const double z){
    if (z > (zbuffer(x,y))) return false;
    else return true;
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

  void draw_tick(int x_tick, int tick_length, string str1, string str2="");

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

