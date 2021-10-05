#include <algorithm>
#include <cassert>
#include <cmath>
#include <iostream>
#include <tuple>
#include <vector>

#include <gd.h>

#include "array2D.hh"
#include "canvas.hh"
#include "geometry.hh"
#include "labelgroup.hh"
#include "mapitems.hh"
#include "scene.hh"
#include "tile.hh"


#pragma omp declare reduction(arraymin                                   \
                              : array2D <double>                         \
                              : omp_out = omp_out.pointwise_min(omp_in)) \
    initializer(omp_priv = array2D <double>(omp_orig))
#pragma omp declare reduction(+                    \
                              : array2D <int32_t>  \
                              : omp_out += omp_in) \
    initializer(omp_priv = array2D <int32_t>(omp_orig))
#pragma omp declare reduction(+                            \
                              : array_zb <double, int32_t> \
                              : omp_out += omp_in)         \
    initializer(omp_priv = array_zb <double, int32_t>(omp_orig))

// input in deg
int get_tile_index(const scene& S, const double lat, const double lon) {
  // find the tile in which the peak is located, continue if none
  int tile_index = -1;
  for (size_t t = 0; t < S.tiles.size(); t++) {
    if (S.tiles[t].first.get_lat() == int(lat) &&
        S.tiles[t].first.get_lon() == int(lon)) {
      tile_index = t;
      break;
    }
  }
  if (tile_index == -1) {
    std::cout << "tile " << int(lat) << "/" << int(lon) << " is required but seems to be inavailable." << std::endl;
  }
  return tile_index;
}

// draw that part of a triangle which is visible
void canvas_t::draw_triangle(const double x1, const double y1,
                             const double x2, const double y2,
                             const double x3, const double y3,
                             const double z,
                             int16_t r, int16_t g, int16_t b) {
  // find triangle's bb
  const int xmin = std::min({std::floor(x1), std::floor(x2), std::floor(x3)});
  const int xmax = std::max({std::ceil(x1), std::ceil(x2), std::ceil(x3)});
  const int ymin = std::min({std::floor(y1), std::floor(y2), std::floor(y3)});
  const int ymax = std::max({std::ceil(y1), std::ceil(y2), std::ceil(y3)});
  const int zero = 0;

  if (xmax - xmin > width() / 2.0)
    return; // avoid drawing triangles that wrap around the edge

  // iterate over grid points in bb, draw the ones in the triangle
  for (int x = std::max(xmin, zero); x < std::min(xmax, width()); x++) {
    for (int y = std::max(ymin, zero); y < std::min(ymax, height()); y++) {
      if (point_in_triangle_2<double>(x + 0.5, y + 0.5, x1, y1, x2, y2, x3, y3)) {
        write_pixel_zb(x, y, z, r, g, b);
      }
    }
  }
}

// true if any pixel was drawn
bool canvas::would_draw_triangle(const double x1, const double y1,
                                 const double x2, const double y2,
                                 const double x3, const double y3,
                                 const double z) const {
  // find triangle's bb
  const int xmin = std::min({std::floor(x1), std::floor(x2), std::floor(x3)});
  const int xmax = std::max({std::ceil(x1), std::ceil(x2), std::ceil(x3)});
  const int ymin = std::min({std::floor(y1), std::floor(y2), std::floor(y3)});
  const int ymax = std::max({std::ceil(y1), std::ceil(y2), std::ceil(y3)});

  bool pixel_drawn = false;
  // iterate over grid points in bb, draw the ones in the triangle
  for (int x = xmin; x < xmax; x++) {
    for (int y = ymin; y < ymax; y++) {
      if (point_in_triangle_2<double>(x + 0.5, y + 0.5, x1, y1, x2, y2, x3, y3)) {
        if (would_write_pixel_zb(x, y, z))
          pixel_drawn = true;
      }
    }
  }
  return pixel_drawn;
}


// draw line if both endpoints are visible,
// return true if drawn
bool canvas::draw_line(const double x1, const double y1,
                       const double x2, const double y2,
                       const double z,
                       int16_t r, int16_t g, int16_t b) {
  if (z - 30 < zbuffer(x1, y1) && z - 30 < zbuffer(x2, y2)) {
    const int colour = gdImageColorResolve(img_ptr, r, g, b);
    gdImageLine(img_ptr, x1, y1, x2, y2, colour);
    return true;
  }
  return false;
}


bool canvas::would_draw_line(const double x1, const double y1,
                             const double x2, const double y2,
                             const double z) const {
  if (z - 30 < zbuffer(x1, y1) && z - 30 < zbuffer(x2, y2)) {
    return true;
  }
  return false;
}


void canvas::draw_tick(int x_tick, int tick_length, const std::string& str1, const std::string& str2) {
  const int black = gdImageColorResolve(img_ptr, 0, 0, 0);
  const double fontsize = 20.;
  char font[] = "./fonts/vera.ttf";
  const double text_orientation = 0;

  // std::cout << "deg90: " << deg << std::endl;
  gdImageLine(img_ptr, x_tick - 1, 0, x_tick - 1, tick_length, black);
  gdImageLine(img_ptr, x_tick, 0, x_tick, tick_length, black);
  gdImageLine(img_ptr, x_tick + 1, 0, x_tick + 1, tick_length, black);

  if (!str1.empty()) {
    // get bb of blank std::string
    int bb[8]; // SW - SE - NE - NW // SW is 0,0
    char* s1 = const_cast<char*>(str1.c_str());
    char* err = gdImageStringFT(nullptr, &bb[0], 0, font, fontsize, 0., 0, 0, s1);
    if (err) {
      fprintf(stderr, "%s", err);
      std::cout << "not good" << std::endl;
    }
    //std::cout << bb[0] << " " << bb[1] << " " << bb[2] << " " << bb[3] << " " << bb[4] << " " << bb[5] << " " << bb[6] << " " << bb[7] << std::endl;

    int xxx = x_tick - bb[2] / 2;
    int yyy = tick_length + 10 - bb[5];
    err = gdImageStringFT(img_ptr, &bb[0],
                          black, font, fontsize, text_orientation,
                          xxx,
                          yyy, s1);
    if (err) {
      fprintf(stderr, "%s", err);
      std::cout << "not good" << std::endl;
    }

    if (!str2.empty()) {
      char* s2 = const_cast<char*>(str2.c_str());
      err = gdImageStringFT(nullptr, &bb[0], 0, font, fontsize, 0., 0, 0, s2);
      if (err) {
        fprintf(stderr, "%s", err);
        std::cout << "not good" << std::endl;
      }
      xxx = x_tick - bb[2] / 2;
      yyy = tick_length + 10 + 20 + 10 - bb[5];
      err = gdImageStringFT(img_ptr, &bb[0],
                            black, font, fontsize, text_orientation,
                            xxx,
                            yyy, s2);
      if (err) {
        fprintf(stderr, "%s", err);
        std::cout << "not good" << std::endl;
      }
    }
  }
}

// always do N, E, S, W
// every 10 deg (always)
// every 5 deg if there are less than 45 deg
// every deg if there are less than 10 deg
void canvas::label_axis(const scene& S) {
  std::cout << "labelling axis ..." << std::flush;
  // const int width = core.get_width();
  const double& view_width = S.view_width;                                                       // [rad]
  const double pixels_per_deg_h = width() / (view_width * rad2deg);                              // [px/deg]
  const double& view_direction_h = S.view_dir_h;                                                 // [rad]
  const double left_border = std::fmod((view_direction_h + view_width / 2), 2 * M_PI) * rad2deg; // [deg]
  // const double right_border = std::fmod((view_direction_h-view_width/2)+2*M_PI,2*M_PI)*rad2deg; // [deg]
  // std::cout << left_border << ", " << right_border << std::endl;

  int deg = std::floor(left_border);                                          // the leftmost std::absolute integer degree on the canvas
  for (int deg_canvas = 0; deg_canvas < view_width * rad2deg; deg_canvas++) { // degree relative to the canvas, from left to right
    const int x_tick = std::fmod(((left_border - deg) + 360), 360) * pixels_per_deg_h;
    if (deg % 90 == 0) {
      // std::cout << "deg90: " << deg << std::endl;
      std::string str1 = std::to_string(deg);
      std::string str2;
      if (deg == 0)
        str2 = "(E)";
      else if (deg == 90)
        str2 = "(N)";
      else if (deg == 180)
        str2 = "(W)";
      else
        str2 = "(S)";
      draw_tick(x_tick, 70, str1, str2);
    }
    else if (deg % 10 == 0) {
      // std::cout << "deg10: " << deg << std::endl;
      std::string str1 = std::to_string(deg);
      draw_tick(x_tick, 70, str1);
    }
    else if (deg % 5 == 0) {
      // std::cout << "deg5: " << deg << std::endl;
      std::string str1 = "";
      if (view_width < 45 * deg2rad) {
        str1 = std::to_string(deg);
      }
      draw_tick(x_tick, 50, str1);
    }
    else {
      // std::cout << "deg1: " << deg << " at " << x_tick << std::endl;
      if (view_width < 45 * deg2rad) {
        std::string str1 = "";
        if (view_width < 10 * deg2rad) {
          str1 = std::to_string(deg);
        }
        draw_tick(x_tick, 50, str1);
      }
    }
    deg--;
    if (deg == -1)
      deg += 360;
  }
  std::cout << " done" << std::endl;
}

void canvas_t::render_scene(const scene& S) {
  std::ofstream debug("debug-render_scene", std::ofstream::out | std::ofstream::app);
  // determine the dimensions, especially pixels/deg
  const double& view_direction_h = S.view_dir_h;         // [rad]
  const double& view_width = S.view_width;               // [rad]
  const double pixels_per_rad_h = width_ / view_width;   // [px/rad]
  const double& view_direction_v = S.view_dir_v;         // [rad]
  const double& view_height = S.view_height;             // [rad]
  const double pixels_per_rad_v = height_ / view_height; // [px/rad]
  debug << "view direction_h [rad]: " << view_direction_h << std::endl;
  debug << "view width [rad]: " << view_width << std::endl;
  debug << "view direction_v [rad]: " << view_direction_v << std::endl;
  debug << "view height [rad]: " << view_height << std::endl;
  debug << "canvas width: " << width_ << std::endl;
  debug << "canvas height: " << height_ << std::endl;
  debug << "horizantal pixels per rad [px/rad]: " << pixels_per_rad_h << std::endl;
  debug << "vertical pixels per rad [px/rad]: " << pixels_per_rad_v << std::endl;
  std::cout << "horizantal pixels per rad [px/rad]: " << pixels_per_rad_h << std::endl;
  std::cout << "vertical pixels per rad [px/rad]: " << pixels_per_rad_v << std::endl;

  // iterate over tiles in scene
#pragma omp parallel for default(none)                                                                                                            \
    shared(S, std::cout, debug, width_, height_, view_width, view_height, view_direction_h, view_direction_v, pixels_per_rad_h, pixels_per_rad_v) \
        reduction(+                                                                                                                               \
                  : buffered_canvas)
  for (size_t t = 0; t < S.tiles.size(); t++) {
    const auto t0 = std::chrono::high_resolution_clock::now();

    const tile<double>& H = S.tiles[t].first;
    const tile<double>& D = S.tiles[t].second;
    const int m = H.get_m();
    const int n = H.get_n();
    // debug << "H: " << H << std::endl;
    // debug << "D: " << D << std::endl;

    debug << "m: " << m << std::endl;
    debug << "n: " << n << std::endl;
    debug << (m - 1) * (n - 1) * 2 << " triangles in tile " << t << std::endl;
    const double invis_angle = std::max(2 * M_PI - view_width, 0.0);
    const int inc = 1;
    for (int i = 0; i < m - inc; i += inc) {
      for (int j = 0; j < n - inc; j += inc) {
        if (D(i, j) > S.view_range)
          continue; // too far
        if (D(i, j) < 100)
          continue; // too close, avoid artifacts
        // first triangle: i/j, i+1/j, i/j+1
        // second triangle: i+1/j, i/j+1, i+1/j+1
        // get horizontal and vertical angles for all four points of the two triangles
        // translate to image coordinates
        bool visible = false; // anything is visible
        const double h_ij = (std::fmod(view_direction_h + view_width / 2.0 + bearing(S.lat_standpoint, S.lon_standpoint, (H.get_lat() + 1 - i / double(m - 1)) * deg2rad, (H.get_lon() + j / double(n - 1)) * deg2rad) + 1.5 * M_PI + invis_angle / 2.0, 2 * M_PI) - invis_angle / 2.0) * pixels_per_rad_h;
        // if(h_ij < 0 || h_ij > width) continue;
        if (h_ij > 0 || h_ij < width_)
          visible = true;
        const double h_ijj = (std::fmod(view_direction_h + view_width / 2.0 + bearing(S.lat_standpoint, S.lon_standpoint, (H.get_lat() + 1 - i / double(m - 1)) * deg2rad, (H.get_lon() + (j + inc) / double(n - 1)) * deg2rad) + 1.5 * M_PI + invis_angle / 2.0, 2 * M_PI) - invis_angle / 2.0) * pixels_per_rad_h;
        // if(h_ijj < 0 || h_ijj > width) continue;
        if (h_ijj > 0 || h_ijj < width_)
          visible = true;
        const double h_iij = (std::fmod(view_direction_h + view_width / 2.0 + bearing(S.lat_standpoint, S.lon_standpoint, (H.get_lat() + 1 - (i + inc) / double(m - 1)) * deg2rad, (H.get_lon() + j / double(n - 1)) * deg2rad) + 1.5 * M_PI + invis_angle / 2.0, 2 * M_PI) - invis_angle / 2.0) * pixels_per_rad_h;
        // if(h_iij < 0 || h_iij > width) continue;
        if (h_iij > 0 || h_iij < width_)
          visible = true;
        const double h_iijj = (std::fmod(view_direction_h + view_width / 2.0 + bearing(S.lat_standpoint, S.lon_standpoint, (H.get_lat() + 1 - (i + inc) / double(m - 1)) * deg2rad, (H.get_lon() + (j + inc) / double(n - 1)) * deg2rad) + 1.5 * M_PI + invis_angle / 2.0, 2.0 * M_PI) - invis_angle / 2.0) * pixels_per_rad_h;
        // if(h_iijj < 0 || h_iijj > width) continue;
        if (h_iijj > 0 || h_iijj < width_)
          visible = true;
        // debug << "("<<i<<","<<j<< ") h: " << h_ij << ", " << h_ijj << ", " << h_iij << ", " << h_iijj << std::endl;

        if (!visible)
          continue;

        // there could be a check here to avoid triangles to wrap around, but
        // it's only tested in draw_triangle

        //std::cout << S.z_standpoint << ", " << H(i,j) << ", " <<  D(i,j) << std::endl;
        const double v_ij = (view_height / 2.0 + view_direction_v - angle_v(S.z_standpoint, H(i, j), D(i, j))) * pixels_per_rad_v; // [px]
        if (v_ij < 0 || v_ij > height_)
          continue;
        const double v_ijj = (view_height / 2.0 + view_direction_v - angle_v(S.z_standpoint, H(i, j + inc), D(i, j + inc))) * pixels_per_rad_v; //[px]
        if (v_ijj < 0 || v_ijj > height_)
          continue;
        const double v_iij = (view_height / 2.0 + view_direction_v - angle_v(S.z_standpoint, H(i + inc, j), D(i + inc, j))) * pixels_per_rad_v; // [px]
        if (v_iij < 0 || v_iij > height_)
          continue;
        const double v_iijj = (view_height / 2.0 + view_direction_v - angle_v(S.z_standpoint, H(i + inc, j + inc), D(i + inc, j + inc))) * pixels_per_rad_v; // [px]
        if (v_iijj < 0 || v_iijj > height_)
          continue;
        // debug << "v: " << v_ij << ", " << v_ijj << ", " << v_iij << ", " << v_iijj << std::endl;
        //std::cout << v_ij << std::endl;

        const double dist1 = (D(i, j) + D(i + inc, j) + D(i, j + inc)) / 3.0;
        // draw_triangle(h_ij, v_ij, h_ijj, v_ijj, h_iij, v_iij, dist1, 5* pow(dist1, 1.0/3.0), H(i,j)*(255.0/3500), 150);
        draw_triangle(h_ij, v_ij, h_ijj, v_ijj, h_iij, v_iij, dist1, 5 * pow(dist1, 1.0 / 3.0), 50, 150);
        // maybe add an array with real heights?
        const double dist2 = (D(i + inc, j) + D(i, j + inc) + D(i + inc, j + inc)) / 3.0;
        // draw_triangle(h_ijj, v_ijj, h_iij, v_iij, h_iijj, v_iijj, dist2, 5*pow(dist1, 1.0/3.0), H(i,j)*(255.0/3500) , 150);
        draw_triangle(h_ijj, v_ijj, h_iij, v_iij, h_iijj, v_iijj, dist2, 5 * pow(dist1, 1.0 / 3.0), 50, 150);
      }
    }

    auto t1 = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> fp_ms = t1 - t0;
    std::cout << "  rendering tile took " << fp_ms.count() << " ms" << std::endl;
  }
  debug.close();
}

// for each column, walk from top to bottom and colour a pixel dark if it is
// much closer than the previous one.  Works only because mountains are
// rarely overhanging or floating in mid-air
void canvas_t::highlight_edges() {
  const auto t0 = std::chrono::high_resolution_clock::now();
  for (int x = 0; x < width_; x++) {
    double z_prev = 1000000;
    for (int y = 0; y < height_; y++) {
      const double z_curr = get_zb(x, y);
      const double thr1 = 1.15, thr2 = 1.05;
      if (z_prev / z_curr > thr1 && z_prev - z_curr > 500) {
        write_pixel(x, y, 0, 0, 0);
      }
      else if (z_prev / z_curr > thr2 && z_prev - z_curr > 200) {
        write_pixel(x, y, 30, 30, 30);
      }
      z_prev = z_curr;
    }
  }

  auto t1 = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double, std::milli> fp_ms = t1 - t0;
  std::cout << "  edge highlighting took " << fp_ms.count() << " ms" << std::endl;
}

void canvas_t::render_test() {
  for (int y = 0; y < height_; y++) {
    for (int x = 0; x < width_; x++) {
      write_pixel(x, y, x, 0.1 * x, y);
    }
  }
}

void canvas_t::bucket_fill(const int r, const int g, const int b) {
  for (int y = 0; y < height_; y++) {
    for (int x = 0; x < width_; x++) {
      const int32_t col = 127 << 24 | r << 16 | g << 8 | b;
      // img_ptr->tpixels[y][x] = col; // assuming TrueColor
      get_wc(x, y) = col;
    }
  }
}

void canvas::annotate_peaks(const scene& S) {
  const auto t0 = std::chrono::high_resolution_clock::now();
  // read all peaks from all tiles in S
  std::vector<point_feature> peaks;
  for (const auto& tile : S.tiles) {
    std::string path("osm");
    std::string xml_name(std::string(tile.first.get_lat() < 0 ? "S" : "N") + to_string_fixedwidth(std::abs(tile.first.get_lat()), 2) +
                         std::string(tile.first.get_lon() < 0 ? "W" : "E") + to_string_fixedwidth(std::abs(tile.first.get_lon()), 3) + "_peak.osm");
    xml_name = path + "/" + xml_name;
    std::vector<point_feature> tmp = read_peaks_osm(xml_name);
    peaks.insert(std::end(peaks), std::begin(tmp), std::end(tmp));
  }
  std::cout << "peaks in db: " << peaks.size() << std::endl;
  // which of those are visible?
  std::vector<point_feature_on_canvas> omitted_peaks;
  auto [visible_peaks, obscured_peaks] = get_visible_peaks(peaks, S);
  std::cout << "number of visible peaks: " << visible_peaks.size() << std::endl;
  std::cout << "number of obscured peaks: " << obscured_peaks.size() << std::endl;
  std::cout << "number of out-of-range/wrong direction peaks: " << peaks.size() - visible_peaks.size() - obscured_peaks.size() << std::endl;
  // label
  omitted_peaks = draw_visible_peaks(visible_peaks);
  std::cout << "number of visible+drawn peaks: " << visible_peaks.size() - omitted_peaks.size() << std::endl;
  std::cout << "number of visible+omitted peaks: " << omitted_peaks.size() << std::endl;
#ifdef GRAPHICS_DEBUG
  draw_invisible_peaks(obscured_peaks, 0, 255, 0);
  draw_invisible_peaks(omitted_peaks, 0, 255, 255);
#endif

  auto t1 = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double, std::milli> fp_ms = t1 - t0;
  std::cout << "  labelling peaks took " << fp_ms.count() << " ms" << std::endl;
}


bool canvas::peak_is_visible_v1(const scene& S, const point_feature& peak, const double dist_peak, const int tile_index) const {
  const double view_direction_h = S.view_dir_h;          // [rad]
  const double view_width = S.view_width;                // [rad]
  const double pixels_per_rad_h = width_ / view_width;   // [px/rad]
  const double view_direction_v = S.view_dir_v;          // [rad]
  const double view_height = S.view_height;              // [rad]
  const double pixels_per_rad_v = height_ / view_height; // [px/rad]

  // integral and fractal part of the point
  double intpart_i, intpart_j;
  const double fractpart_i = std::modf(peak.lat, &intpart_i);
  const double fractpart_j = std::modf(peak.lon, &intpart_j);

  const tile<double>& H = S.tiles[tile_index].first;
  const tile<double>& D = S.tiles[tile_index].second;
  const int m = H.get_m();
  const int n = H.get_n();

  // get a few triangles around the peak, we're interested in 25 squares around the peak, between i-rad/j-rad and i+rad/j+rad
  // the test-patch should be larger for large distances because there are less pixels per ground area
  const int radius = 2 + dist_peak * pixels_per_rad_h / (1.0 * 10000000); // the numbers are chosen because they sort-of work
  const int diameter = 2 * radius + 1;
  // std::cout << dist_peak << ", " << radius << ", " << diameter << std::endl;

  const int tile_size_m1 = S.tiles[tile_index].first.get_dim() - 1; // because we always need size-1 here

  // ii and jj pont to the NW corner of a 5x5 grid of tiles where the feature is in the middle tile
  const int ii = tile_size_m1 - std::ceil(std::abs(fractpart_i) * (tile_size_m1)) - radius; // lat
  const int jj = std::floor(std::abs(fractpart_j) * tile_size_m1) - radius;                 // lon
  //      std::cout << "ii,jj: " <<  ii << ", " << jj << std::endl;
  //      std::cout << "coords: " << peaks[p].get_lat() << ", " << peaks[p].get_lon() << std::endl;
  //      std::cout << "points will be at: " << 1-ii/3600.0 << ", " << 1-(ii+1)/3600.0 << ", " << 1-(ii+2)/3600.0 << ", " << 1-(ii+3)/3600.0 << std::endl;
  //      std::cout << "points will be at: " << jj/3600.0 << ", " << (jj+1)/3600.0 << ", " << (jj+2)/3600.0 << ", " << (jj+3)/3600.0 << std::endl;

  // test if peak would be rendered, hence, is visible
#ifdef GRAPHICS_DEBUG
  bool visible = false;
#endif
  const int inc = 1;
  for (int i = ii; i < ii + diameter; i++) {
    if (i < 0 || i > tile_size_m1)
      continue; // FIXME
    for (int j = jj; j < jj + diameter; j++) {
      if (j < 0 || j > tile_size_m1)
        continue; // FIXME
      const double h_ij = std::fmod(view_direction_h + view_width / 2.0 + bearing(S.lat_standpoint, S.lon_standpoint, (H.get_lat() + 1 - i / double(m - 1)) * deg2rad, (H.get_lon() + j / double(n - 1)) * deg2rad) + 1.5 * M_PI, 2 * M_PI) * pixels_per_rad_h;
      if (h_ij < 0 || h_ij > width_)
        continue;
      const double h_ijj = std::fmod(view_direction_h + view_width / 2.0 + bearing(S.lat_standpoint, S.lon_standpoint, (H.get_lat() + 1 - i / double(m - 1)) * deg2rad, (H.get_lon() + (j + inc) / double(n - 1)) * deg2rad) + 1.5 * M_PI, 2 * M_PI) * pixels_per_rad_h;
      if (h_ijj < 0 || h_ijj > width_)
        continue;
      const double h_iij = std::fmod(view_direction_h + view_width / 2.0 + bearing(S.lat_standpoint, S.lon_standpoint, (H.get_lat() + 1 - (i + inc) / double(m - 1)) * deg2rad, (H.get_lon() + j / double(n - 1)) * deg2rad) + 1.5 * M_PI, 2 * M_PI) * pixels_per_rad_h;
      if (h_iij < 0 || h_iij > width_)
        continue;
      const double h_iijj = std::fmod(view_direction_h + view_width / 2.0 + bearing(S.lat_standpoint, S.lon_standpoint, (H.get_lat() + 1 - (i + inc) / double(m - 1)) * deg2rad, (H.get_lon() + (j + inc) / double(n - 1)) * deg2rad) + 1.5 * M_PI, 2.0 * M_PI) * pixels_per_rad_h;
      if (h_iijj < 0 || h_iijj > width_)
        continue;

      //std::cout << S.z_standpoint << ", " << H(i,j) << ", " <<  D(i,j) << std::endl;
      const double v_ij = (view_height / 2.0 + view_direction_v - angle_v(S.z_standpoint, H(i, j), D(i, j))) * pixels_per_rad_v; // [px]
      if (v_ij < 0 || v_ij > height_)
        continue;
      const double v_ijj = (view_height / 2.0 + view_direction_v - angle_v(S.z_standpoint, H(i, j + inc), D(i, j + inc))) * pixels_per_rad_v; //[px]
      if (v_ijj < 0 || v_ijj > height_)
        continue;
      const double v_iij = (view_height / 2.0 + view_direction_v - angle_v(S.z_standpoint, H(i + inc, j), D(i + inc, j))) * pixels_per_rad_v; // [px]
      if (v_iij < 0 || v_iij > height_)
        continue;
      const double v_iijj = (view_height / 2.0 + view_direction_v - angle_v(S.z_standpoint, H(i + inc, j + inc), D(i + inc, j + inc))) * pixels_per_rad_v; // [px]
      if (v_iijj < 0 || v_iijj > height_)
        continue;
      //debug << "v: " << v_ij << ", " << v_ijj << ", " << v_iij << ", " << v_iijj << std::endl;

      const double dist1 = (D(i, j) + D(i + inc, j) + D(i, j + inc)) / 3.0 - 2;
      const double dist2 = (D(i + inc, j) + D(i, j + inc) + D(i + inc, j + inc)) / 3.0 - 2;
      if (would_draw_triangle(h_ij, v_ij, h_ijj, v_ijj, h_iij, v_iij, dist1))
#ifdef GRAPHICS_DEBUG
        visible = true;
#else
        return true;
#endif
      if (would_draw_triangle(h_ijj, v_ijj, h_iij, v_iij, h_iijj, v_iijj, dist2))
#ifdef GRAPHICS_DEBUG
        visible = true;
#else
        return true;
#endif
#ifdef GRAPHICS_DEBUG
      draw_triangle(h_ij, v_ij, h_ijj, v_ijj, h_iij, v_iij, dist1, 5 * pow(dist1, 1.0 / 3.0), H(i, j) * (255.0 / 3500), 50);
      draw_triangle(h_ijj, v_ijj, h_iij, v_iij, h_iijj, v_iijj, dist2, 5 * pow(dist1, 1.0 / 3.0), H(i, j) * (255.0 / 3500), 250);
#endif
    }
  }
#ifdef GRAPHICS_DEBUG
  if (visible)
    return true;
#endif
  return false;
}


// test if a peak is visible by drawing a line on the ground from the peak to
// the viewer, but only downhill.  If any part of the line is drawn, I call the
// peak visible.  This avoids the problem of flat topped mountains and works
// only under the condition that mountains don't float in midair.
bool canvas::peak_is_visible_v2(const scene& S, const point_feature& peak, const double dist_peak) const {
  const double& view_direction_h = S.view_dir_h;         // [rad]
  const double& view_width = S.view_width;               // [rad]
  const double pixels_per_rad_h = width_ / view_width;   // [px/rad]
  const double& view_direction_v = S.view_dir_v;         // [rad]
  const double& view_height = S.view_height;             // [rad]
  const double pixels_per_rad_v = height_ / view_height; // [px/rad]
  const double ref_lat = S.lat_standpoint;
  const double ref_lon = S.lon_standpoint;

  // bearing to peak
  // std::cout << ref_lat << "," << ref_lon <<", "<< peak.get_lat() <<","<< peak.get_lon() << std::endl;
  const double bearing_rad = bearing(ref_lat, ref_lon, peak.lat * deg2rad, peak.lon * deg2rad);
  // std::cout << "bearing " << bearing_rad << " / " << bearing_rad*rad2deg << std::endl;

  // chose increment, calc number of steps
  const double seg_length = 30.0; // [m]
  const double n_segs = dist_peak / seg_length;
  // std::cout << "seg no/length: " << n_segs << ", " << seg_length << std::endl;

  double prev_height = 10000.0;  // [m]
  double prev_x = 1, prev_y = 1; // [px]

#ifdef GRAPHICS_DEBUG
  bool visible = false;
#endif
  for (int seg = 0; seg < n_segs; seg++) {
    // std::cout << "seg no: " << seg << std::endl;
    // get coords of segment endpoints
    const double dist_point = dist_peak - seg_length * seg;
    auto [point_lat, point_lon] = destination(ref_lat, ref_lon, dist_point, bearing_rad);

    // find tile in which the point lies
    const int tile_index = get_tile_index(S, point_lat * rad2deg, point_lon * rad2deg);
    // std::cout << "point coord: " << point_lat*rad2deg << ", " <<  point_lon*rad2deg << std::endl;
    // std::cout << "tile index: " << tile_index << std::endl;
    const tile<double>& H = S.tiles[tile_index].first;

    // interpolate to get elevation
    const double height_point = H.interpolate(point_lat * rad2deg, point_lon * rad2deg);
    // std::cout << "height: " << height_point << std::endl;
    // if uphill, but allow for slightly wrong peak location
    if (height_point > prev_height && seg > 2) {
      // std::cout << "uphill, leaving" << std::endl;
      break;
    }

    // get coords on canvas
    // std::cout << view_direction_h << ", " <<  view_width/2.0 << ", " <<  bearing_rad << ", " << 1.5*M_PI << std::endl;
    const double x_point = std::fmod(view_direction_h + view_width / 2.0 + bearing_rad + 1.5 * M_PI, 2 * M_PI) * pixels_per_rad_h;        // [px]
    const double y_point = (view_direction_v + view_height / 2.0 - angle_v(S.z_standpoint, height_point, dist_point)) * pixels_per_rad_v; // [px]
    // std::cout << "point x,y " << x_point << ", " << y_point << std::endl;

    // draw?
    // std::cout << prev_x << ", " << prev_y << ", " << x_point << ", " << y_point << ", " << dist_point+seg_length/2.0 << std::endl;
    if (seg > 0) {
#ifdef GRAPHICS_DEBUG
      const int r(0), g(255), b(0);
      visible |= draw_line(prev_x, prev_y, x_point, y_point, dist_point + seg_length / 2.0, r, g, b);
#else
      if (would_draw_line(prev_x, prev_y, x_point, y_point, dist_point + seg_length / 2.0))
        return true;
#endif
    }
    prev_x = x_point;
    prev_y = y_point;
    prev_height = height_point;
  }

#ifdef GRAPHICS_DEBUG
  //std::cout << "visible true" << std::endl;
  if (visible)
    return true;
#endif
  return false;
}


// test if a peak is visible by attempting to draw a few triangles around it,
// if the zbuffer admits any pixel to be drawn, the peak is visible
std::tuple<std::vector<point_feature_on_canvas>, std::vector<point_feature_on_canvas>> canvas::get_visible_peaks(std::vector<point_feature>& peaks, const scene& S) {
  const double view_direction_h = S.view_dir_h;          // [rad]
  const double view_width = S.view_width;                // [rad]
  const double pixels_per_rad_h = width_ / view_width;   // [px/rad]
  const double view_direction_v = S.view_dir_v;          // [rad]
  const double view_height = S.view_height;              // [rad]
  const double pixels_per_rad_v = height_ / view_height; // [px/rad]
  // std::cout << "pprh: " << pixels_per_rad_h << std::endl;

  std::vector<point_feature_on_canvas> visible_peaks;
  std::vector<point_feature_on_canvas> obscured_peaks;
  for (size_t p = 0; p < peaks.size(); p++) {
    // std::cout << "--- p=" << p << " ---" << std::endl;
    // distance from the peak
    const double dist_peak = distance_atan(S.lat_standpoint, S.lon_standpoint, peaks[p].lat * deg2rad, peaks[p].lon * deg2rad);
    if (dist_peak > S.view_range || dist_peak < 1000)
      continue;

    const int tile_index = get_tile_index(S, peaks[p].lat, peaks[p].lon);
    if (tile_index == -1) {
      std::cout << "skip" << std::endl;
      continue;
    }

    const tile<double>& H = S.tiles[tile_index].first;

    // height of the peak, according to elevation data
    const double height_peak = H.interpolate(peaks[p].lat, peaks[p].lon);
    // std::cout << "peak height and dist: " << height_peak << ", " << dist_peak << std::endl;
    // if the osm doesn't know the height, take from elevation data
    const double coeff = 0.065444 / 1000000.0; // = 0.1695 / 1.609^2  // m
    if (peaks[p].elev == 0)
      peaks[p].elev = height_peak + coeff * pow(dist_peak, 2); // revert earth's curvature

    // get position of peak on canvas, continue if outside
    const double x_peak = std::fmod(view_direction_h + view_width / 2.0 + bearing(S.lat_standpoint, S.lon_standpoint, peaks[p].lat * deg2rad, peaks[p].lon * deg2rad) + 1.5 * M_PI, 2 * M_PI) * pixels_per_rad_h;
    const double y_peak = (view_direction_v + view_height / 2.0 - angle_v(S.z_standpoint, height_peak, dist_peak)) * pixels_per_rad_v; // [px]
    // std::cout << "peak x, y " << x_peak << ", " << y_peak << std::endl;
    if (x_peak < 0 || x_peak > width_)
      continue;
    if (y_peak < 0 || y_peak > height_)
      continue;

    //if(peak_is_visible_v1(S, peaks[p], dist_peak, tile_index))
    if (peak_is_visible_v2(S, peaks[p], dist_peak))
      visible_peaks.emplace_back(peaks[p], x_peak, y_peak, dist_peak);
    else
      obscured_peaks.emplace_back(peaks[p], x_peak, y_peak, dist_peak);
  }
  return {visible_peaks, obscured_peaks};
}


std::vector<point_feature_on_canvas> canvas::draw_visible_peaks(const std::vector<point_feature_on_canvas>& peaks_vis) {
  //int n_labels = peaks_vis.size();

  LabelGroups lgs(peaks_vis, width_);

  // prune ... if the offsets in one group get too large, some of the lower peaks should be omitted
  std::vector<point_feature_on_canvas> omitted_peaks = lgs.prune();

  for (size_t p = 0; p < lgs.size(); p++) {
    const int& x_peak = lgs[p].x;
    const int& y_peak = lgs[p].y;
    const int& dist_peak = lgs[p].dist;
    // std::cout << peaks[p].name << " is visible" << std::endl;
    // std::cout << "pixel will be written at : " << x_peak << ", " << y_peak << std::endl;
    write_pixel(x_peak, y_peak, 255, 0, 0);

    // const int x_offset=0;
    const int y_offset = 100;

    const int black = gdImageColorResolve(img_ptr, 0, 0, 0);
    gdImageLine(img_ptr, x_peak, y_peak - 2, x_peak + lgs[p].xshift, y_peak - y_offset + 5, black);

    std::string name(lgs[p].pf.name);
    if (!lgs[p].pf.name.empty())
      name += ", ";
    name += std::to_string(lgs[p].pf.elev) + "m, " + std::to_string(int(std::round(dist_peak / 1000))) + "km";
    char* s = const_cast<char*>(name.c_str());
    const double fontsize = 12.;
    //char font[] = "./palatino-59330a4da3d64.ttf";
    char font[] = "./fonts/vera.ttf";
    const double text_orientation = M_PI / 2;

    // get bb of blank string
    int bb[8];
    char* err = gdImageStringFT(nullptr, &bb[0], 0, font, fontsize, 0., 0, 0, s);
    if (err) {
      fprintf(stderr, "%s", err);
      std::cout << "not good" << std::endl;
    }
    //      std::cout << bb[0] << " " << bb[1] << " " << bb[2] << " " << bb[3] << " " << bb[4] << " " << bb[5] << " " << bb[6] << " " << bb[7] << std::endl;

    err = gdImageStringFT(img_ptr, &bb[0],
                          black, font, fontsize, text_orientation,
                          x_peak + lgs[p].xshift + fontsize / 2.0,
                          y_peak - y_offset, s);
    if (err) {
      fprintf(stderr, "%s", err);
      std::cout << "not good" << std::endl;
    }
  }
  // print number drawn and number omitted

  return omitted_peaks;
}


void canvas::draw_invisible_peaks(const std::vector<point_feature_on_canvas>& peaks_invis,
                                  const int16_t r, const int16_t g, const int16_t b) {
  for (size_t p = 0; p < peaks_invis.size(); p++) {
    std::cout << peaks_invis[p].pf.name << " is invisible" << std::endl;
    std::cout << "pixel will be written at : " << peaks_invis[p].x << ", " << peaks_invis[p].y << std::endl;
    write_pixel(peaks_invis[p].x, peaks_invis[p].y, r, g, b);
  }
}

void canvas::annotate_islands(const scene& S) {
  // read all peaks from all tiles in S
  std::vector<linear_feature> islands;
  // for (auto it=S.tiles.begin(), to=S.tiles.end(); it!=to; it++){
  //   std::string path("osm");
  //   std::string xml_name(std::string(it->first.get_lat()<0?"S":"N") + to_string_fixedwidth(std::abs(it->first.get_lat()),2) +
  //                   std::string(it->first.get_lon()<0?"W":"E") + to_string_fixedwidth(std::abs(it->first.get_lon()),3) + "_isl.osm");
  //   xml_name = path + "/" + xml_name;
  //   std::vector<linear_feature> tmp = read_islands_osm(xml_name);
  //   islands.insert(std::end(islands), std::begin(tmp), std::end(tmp));
  // }
}

void canvas::draw_coast(const scene& S) {
  // const int width(core.get_width()),
  //     height(core.get_height());
  // read all peaks from all tiles in S
  std::vector<linear_feature> coasts;
  for (const auto& T : S.tiles) {
    std::string path("osm");
    std::string xml_name(std::string(T.first.get_lat() < 0 ? "S" : "N") + to_string_fixedwidth(std::abs(T.first.get_lat()), 2) +
                         std::string(T.first.get_lon() < 0 ? "W" : "E") + to_string_fixedwidth(std::abs(T.first.get_lon()), 3) + "_coast.osm");
    xml_name = path + "/" + xml_name;
    std::vector<linear_feature> tmp = read_coast_osm(xml_name);
    coasts.insert(std::end(coasts), std::begin(tmp), std::end(tmp));
  }

  std::cout << "coasts" << std::endl;
  std::cout << coasts << std::endl;

  // remove duplicates
  // FIXME, switch to std::set from the start
  std::cout << "size1: " << coasts.size() << std::endl;
  std::set<linear_feature> tmp(coasts.begin(), coasts.end());
  coasts.assign(tmp.begin(), tmp.end());
  std::cout << "size2: " << coasts.size() << std::endl;
  for (const auto& lf : coasts) {
    std::cout << "coast ids: " << lf.id << std::endl;
  }

  // get linear feature on canvas
  std::set<linear_feature_on_canvas> coasts_oc;
  for (const linear_feature& coast : coasts) {

    linear_feature_on_canvas lfoc_tmp(coast, *this, S);
    std::cout << lfoc_tmp.lf.id << std::endl;
    coasts_oc.insert(lfoc_tmp);
    std::cout << "b" << std::endl;
  }


  // do the actual drawing
  for (const linear_feature_on_canvas& coast_oc : coasts_oc) {
    std::cout << "new line feature" << std::endl;
    for (int p = 0; p < static_cast<int>(coast_oc.size() - 1); p++) {
      const int
          x1 = coast_oc.xs[p],
          y1 = coast_oc.ys[p], z = coast_oc.dists[p],
          x2 = coast_oc.xs[p + 1], y2 = coast_oc.ys[p + 1]; // z2 = coast.dists[p+1];
      std::cout << x1 << ", " << y1 << ", " << x2 << ", " << y2 << std::endl;
      //      std::cout << x1 << ", " << y1 << ", " << x2 << ", " << y2 << ", " << z << std::endl;
      if (x1 < 0 || x1 > width_)
        continue;
      if (x2 < 0 || x2 > width_)
        continue;
      if (y1 < 0 || y1 > height_)
        continue;
      if (y2 < 0 || y2 > height_)
        continue;
      const int r(0), g(255), b(0);
      std::cout << " drawing line" << std::endl;
      draw_line(x1, y1, x2, y2, z, r, g, b);
    }
  }
}
