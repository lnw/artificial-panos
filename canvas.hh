#ifndef CANVAS_HH
#define CANVAS_HH

#include <vector>
#include <iostream>
#include <math.h>
#include <algorithm> // min, max
#include <cmath> // modf

#include <gd.h>

#include "geometry.hh"
#include "array2D.hh"
#include "tile.hh"
#include "scene.hh"
#include "colour.hh"
#include "mapitems.hh"

using namespace std;

class canvas {

public:
  int width, height; // [pixels]
  array2D<double> zbuffer; // initialised to 1000 km [m]

private:
  char const * filename;
  gdImagePtr img_ptr = nullptr;

public:
  canvas(char const * fn, int x, int y): width(x), height(y), zbuffer(x,y,1000000), filename(fn){
    // allocate mem
    img_ptr = gdImageCreateTrueColor(width, height);
  }

  ~canvas(){
    // actually the file is only opened here
    FILE *png_ptr = fopen(filename, "wb");
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
    // const int col = gdImageColorAllocate(img_ptr, r, g, b);
    // gdImageSetPixel(img_ptr, x, y, col);
  }

  // just write the pixel taking into account the zbuffer
  // true if pixel was drawn
  void write_pixel_zb(const int x, const int y, const double z,
                      int16_t r, int16_t g, int16_t b){
    if (z < zbuffer(x,y)){
      zbuffer(x,y) = z;
      const int32_t col = 127 << 24 | r << 16 | g << 8 | b ;
      img_ptr->tpixels[y][x] = col; // assuming TrueColor
      // const int col = gdImageColorAllocate(img_ptr, r, g, b);
      // gdImageSetPixel(img_ptr, x, y, col);
    }
  }

  // just write the pixel taking into account the zbuffer
  // true if pixel was drawn
  bool would_write_pixel_zb(const int x, const int y, const double z){
    if (z > (zbuffer(x,y))) return false;
    else return true;
  }

  // true if any pixel was drawn
  void write_triangle(const double x1, const double y1,
                      const double x2, const double y2,
                      const double x3, const double y3,
                      const double z,
                      int16_t r, int16_t g, int16_t b){
    // find triangle's bb
    const int xmin = min( {floor(x1), floor(x2), floor(x3)} );
    const int xmax = max( {ceil(x1),  ceil(x2),  ceil(x3)} );
    const int ymin = min( {floor(y1), floor(y2), floor(y3)} );
    const int ymax = max( {ceil(y1),  ceil(y2),  ceil(y3)} );

    // iterate over grid points in bb, draw the ones in the triangle
    for (size_t x=xmin; x<xmax; x++){
      for (size_t y=ymin; y<ymax; y++){
        if(point_in_triangle_2 (x+0.5,y+0.5, x1,y1,x2,y2,x3,y3)){
          write_pixel_zb(x,y,z, r,g,b);
        }
      }
    }
  }

  // true if any pixel was drawn
  bool would_write_triangle(const double x1, const double y1,
                            const double x2, const double y2,
                            const double x3, const double y3,
                            const double z){
    // find triangle's bb
    const int xmin = min( {floor(x1), floor(x2), floor(x3)} );
    const int xmax = max( {ceil(x1),  ceil(x2),  ceil(x3)} );
    const int ymin = min( {floor(y1), floor(y2), floor(y3)} );
    const int ymax = max( {ceil(y1),  ceil(y2),  ceil(y3)} );

    bool pixel_drawn=false;
    // iterate over grid points in bb, draw the ones in the triangle
    for (size_t x=xmin; x<xmax; x++){
      for (size_t y=ymin; y<ymax; y++){
        if(point_in_triangle_2 (x+0.5,y+0.5, x1,y1,x2,y2,x3,y3)){
          if(would_write_pixel_zb(x,y,z)) pixel_drawn = true;
        }
      }
    }
    return pixel_drawn;
  }

//  // only for dx >= dy
//  void write_line(const double x1, const double y1,
//                  const double x2, const double y2,
//                  const double z,
//                  int16_t r, int16_t g, int16_t b, int16_t a){
//    const int Dx = x2-x1, Dy = y2-y1;
//    int d = 2*Dy-Dx;
//    const int DE = 2*Dy; // east
//    const int DNE = 2*(Dy-Dx); // north east
//
//    if(abs(DE) > abs(DNE)) return;
//    int y = y1;
//    write_pixel_zb(x1,y1,z, r,g,b);
//    for(int x=x1+1; x<x2; x++){
//      if(d <= 0){
//        d += DE;
//      }else{
//        d += DNE;
//        y++;
//      }
//      write_pixel_zb(x,y,z, r,g,b);
//    }
//  }

//  void write_tick_top(const double x, const double y, const int lw,
//                      const double z,
//                      int16_t r, int16_t g, int16_t b, int16_t a){
//    for(int i=x-lw/2; i<x+lw/2; i++){
//      for(int j=1; j<y; j++){
//        write_pixel_zb(i,j,z, r,g,b);
//      }
//    }
//  }


  void render_scene(const scene& S){
    ofstream debug("debug-render_scene", ofstream::out | ofstream::app);
    // determine the dimensions, especially pixels/deg
    const double& view_direction_h = S.view_dir_h; // [rad]
    const double& view_width = S.view_width; // [rad]
    const double pixels_per_rad_h = width / view_width; // [px/rad]
    const double& view_direction_v = S.view_dir_v; // [rad]
    const double& view_height = S.view_height; // [rad]
    const double pixels_per_rad_v = height / view_height; // [px/rad]
    debug << "view direction_h [rad]: " << view_direction_h << endl;
    debug << "view width [rad]: " << view_width << endl;
    debug << "view direction_v [rad]: " << view_direction_v << endl;
    debug << "view height [rad]: " << view_height << endl;
    debug << "canvas width: " << width << endl;
    debug << "canvas height: " << height << endl;
    debug << "horizantal pixels per rad [px/rad]: " << pixels_per_rad_h << endl;
    debug << "vertical pixels per rad [px/rad]: " << pixels_per_rad_v << endl;

    //iterate over tiles in scene
    for (size_t t=0; t<S.tiles.size(); t++){
      const tile<double> &H = S.tiles[t].first;
      const tile<double> &D = S.tiles[t].second;
      const int &m = H.m;
      const int &n = H.n;
      // debug << "H: " << H << endl;
      // debug << "D: " << D << endl;

      debug << "m: " << m << endl;
      debug << "n: " << n << endl;
      debug << (m-1)*(n-1)*2 << " triangles in tile " << t << endl;
      const int inc = 1;
      for (size_t i=0; i<m-inc; i+=inc){
        for (size_t j=0; j<n-inc; j+=inc){
          if(D(i,j) > S.view_dist) continue;
          if(D(i,j) < 100) continue; // avoid close artifacts
          // first triangle: i/j, i+1/j, i/j+1
          // second triangle: i+1/j, i/j+1, i+1/j+1
          // get horizontal and vertical angles for all four points of the two triangles
          // translate to image coordinates
          const double h_ij = fmod(view_direction_h + view_width/2.0 + bearing(S.lat_standpoint, S.lon_standpoint, (H.lat + 1 - i/double(m-1))*deg2rad, (H.lon + j/double(n-1))*deg2rad) + 1.5*M_PI, 2*M_PI) * pixels_per_rad_h;
          if(h_ij < 0 || h_ij > width) continue;
          const double h_ijj = fmod(view_direction_h + view_width/2.0 + bearing(S.lat_standpoint, S.lon_standpoint, (H.lat + 1 - i/double(m-1))*deg2rad, (H.lon + (j+inc)/double(n-1))*deg2rad) + 1.5*M_PI, 2*M_PI) * pixels_per_rad_h;
          if(h_ijj < 0 || h_ijj > width) continue;
          const double h_iij = fmod(view_direction_h + view_width/2.0 + bearing(S.lat_standpoint, S.lon_standpoint, (H.lat + 1 - (i+inc)/double(m-1))*deg2rad, (H.lon + j/double(n-1))*deg2rad) + 1.5*M_PI ,2*M_PI) * pixels_per_rad_h;
          if(h_iij < 0 || h_iij > width) continue;
          const double h_iijj = fmod(view_direction_h + view_width/2.0 + bearing(S.lat_standpoint, S.lon_standpoint, (H.lat + 1 - (i+inc)/double(m-1))*deg2rad, (H.lon + (j+inc)/double(n-1))*deg2rad) + 1.5*M_PI, 2.0*M_PI) * pixels_per_rad_h;
          if(h_iijj < 0 || h_iijj > width) continue;
          //debug << "("<<i<<","<<j<< ") h: " << h_ij << ", " << h_ijj << ", " << h_iij << ", " << h_iijj << endl;

          // there should be a check here to avoid triangles to wrap around

          //cout << S.z_standpoint << ", " << H(i,j) << ", " <<  D(i,j) << endl;
          const double v_ij   = (view_height/2.0 + view_direction_v - angle_v(S.z_standpoint, H(i,j), D(i,j))) * pixels_per_rad_v; // [px]
          if(v_ij < 0 || v_ij > height) continue;
          const double v_ijj  = (view_height/2.0 + view_direction_v - angle_v(S.z_standpoint, H(i,j+inc), D(i,j+inc))) * pixels_per_rad_v; //[px]
          if(v_ijj < 0 || v_ijj > height) continue;
          const double v_iij  = (view_height/2.0 + view_direction_v - angle_v(S.z_standpoint, H(i+inc,j), D(i+inc,j))) * pixels_per_rad_v; // [px]
          if(v_iij < 0 || v_iij > height) continue;
          const double v_iijj = (view_height/2.0 + view_direction_v - angle_v(S.z_standpoint, H(i+inc,j+inc), D(i+inc,j+inc))) * pixels_per_rad_v; // [px]
          if(v_iijj < 0 || v_iijj > height) continue;
          //debug << "v: " << v_ij << ", " << v_ijj << ", " << v_iij << ", " << v_iijj << endl;
          //cout << v_ij << endl;

          const double dist1 = (D(i,j)+D(i+inc,j)+D(i,j+inc))/3.0;
          // write_triangle(h_ij, v_ij, h_ijj, v_ijj, h_iij, v_iij, dist1, D(i,j)*(255.0/S.view_dist), H(i,j)*(255.0/3500), 150, 255);
          write_triangle(h_ij, v_ij, h_ijj, v_ijj, h_iij, v_iij, dist1, 5* pow(dist1, 1.0/3.0), H(i,j)*(255.0/3500), 150);
          const double dist2 = (D(i+inc,j)+D(i,j+inc)+D(i+inc,j+inc))/3.0;
          // write_triangle(h_ijj, v_ijj, h_iij, v_iij, h_iijj, v_iijj, dist2, D(i,j)*(255.0/S.view_dist), H(i,j)*(255.0/3500) , 150, 255);
          write_triangle(h_ijj, v_ijj, h_iij, v_iij, h_iijj, v_iijj, dist2, 5*pow(dist1, 1.0/3.0), H(i,j)*(255.0/3500) , 150);
        }
      }
    }
    debug.close();
  }

  // for each column, walk from top to bottom and colour a pixel dark if it is
  // much closer than the previous one.  Works only because mountains are
  // rarely overhanging
  void highlight_edges(){
    for(size_t x=0; x<width; x++){
      double z_prev = 1000000;
      for(size_t y=0; y<height; y++){
        const double z_curr = zbuffer(x,y);
        const double thr1 = 1.15, thr2 = 1.05;
        if(z_prev / z_curr > thr1 && z_prev - z_curr > 500){
        // if(z_prev / z_curr > thr1){
          write_pixel(x,y, 0,0,0);
        }
        else if(z_prev / z_curr > thr2 && z_prev - z_curr > 300){
          write_pixel(x,y, 30,30,30);
        }
        z_prev = z_curr;
      }
    }
  }

  void render_test(){
    for (size_t y=0; y<height; y++) {
      for (size_t x=0; x<width; x++) {
        write_pixel(x,y, x,0.1*x,y);
      }
    }
  }

  void bucket_fill( const int r, const int g, const int b){
    for (size_t y=0; y<height; y++) {
      for (size_t x=0; x<width; x++) {
        const int32_t col = 127 << 24 | r << 16 | g << 8 | b ;
        img_ptr->tpixels[y][x] = col; // assuming TrueColor
      }
    }
  }

  void annotate_peaks(const scene& S, const char * xml_name){
    // get list of peaks
    vector<point_feature> peaks = read_peaks_osm(xml_name);
    cout << "peaks in db: " << peaks.size() << endl;

    const double& view_direction_h = S.view_dir_h; // [rad]
    const double& view_width = S.view_width; // [rad]
    const double pixels_per_rad_h = width / view_width; // [px/rad]
    const double& view_direction_v = S.view_dir_v; // [rad]
    const double& view_height = S.view_height; // [rad]
    const double pixels_per_rad_v = height / view_height; // [px/rad]

    for(size_t p=0; p<peaks.size(); p++){
      // distance from the peak
      const double dist_peak = distance_atan(S.lat_standpoint, S.lon_standpoint, peaks[p].lat*deg2rad, peaks[p].lon*deg2rad);
      if(dist_peak > S.view_dist || dist_peak < 1000) continue;

      // the test-patch should be larger for large distances because there are less pixels per ground area
      const int radius = 2 + dist_peak/20000;
      const int diameter = 2*radius+1;
      cout << dist_peak << ", " << radius << ", " << diameter << endl;

      // get a few triangles around the peak, we're interested in 25 squares around the peak, between i/j and i+6/j+6
      double intpart_i, intpart_j;
      const double fractpart_i = modf (peaks[p].lat, &intpart_i), fractpart_j = modf (peaks[p].lon, &intpart_j);
      // ii and jj pont to the NW corner of a 5x5 grid of tiles where the feature is in the middle tile
      const int ii = 3600 - ceil(abs(fractpart_i)*3600) - radius; // lat
      const int jj = floor(abs(fractpart_j)*3600) - radius; // lon
//      cout << "ii,jj: " <<  ii << ", " << jj << endl;
//      cout << "coords: " << peaks[p].lat << ", " << peaks[p].lon << endl;
//      cout << "points will be at: " << 1-ii/3600.0 << ", " << 1-(ii+1)/3600.0 << ", " << 1-(ii+2)/3600.0 << ", " << 1-(ii+3)/3600.0 << endl;
//      cout << "points will be at: " << jj/3600.0 << ", " << (jj+1)/3600.0 << ", " << (jj+2)/3600.0 << ", " << (jj+3)/3600.0 << endl;

      // find the tile in which the peak is located, continue if none
      int tile_index = -1;
      for(size_t t = 0; t<S.tiles.size(); t++){
        if(S.tiles[t].first.lat == round(intpart_i) &&
           S.tiles[t].first.lon == round(intpart_j)){
          tile_index = t;
          break;
        }
      }
      if(tile_index == -1){
        cout << "tile " << round(intpart_i) << "/" << round(intpart_j) << " is required but seems to be inavailable.  skipping a peak." << endl;
        continue;
      }
      const tile<double> &H = S.tiles[tile_index].first;
      const tile<double> &D = S.tiles[tile_index].second;
      const int &m = H.m;
      const int &n = H.n;

      // height of the peak, according to elevation data
      const double height_peak = H.interpolate(peaks[p].lat, peaks[p].lon);
      cout << "peak height and dist: " << height_peak << ", " << dist_peak << endl;
      // if the osm doesn't know the height, take from elevation data
      if(peaks[p].elev == 0) peaks[p].elev = height_peak;

      // get position of peak on canvas, continue if outside
      const double x_peak = fmod(view_direction_h + view_width/2.0 + bearing(S.lat_standpoint, S.lon_standpoint, peaks[p].lat*deg2rad, peaks[p].lon*deg2rad) + 1.5*M_PI, 2*M_PI) * pixels_per_rad_h;
      const double y_peak = (view_height/2.0 + view_direction_v - angle_v(S.z_standpoint, height_peak, dist_peak)) * pixels_per_rad_v; // [px]
      cout << "peak x, y " << x_peak << ", " << y_peak << endl;
      if(x_peak < 0 || x_peak > width ) continue;
      if(y_peak < 0 || y_peak > height ) continue;

      // test if peak would be rendered, hence, is visible
      const int inc=1;
      bool peak_visible=false;
      for(int i=ii; i<ii+diameter; i++){
        for(int j=jj; j<jj+diameter; j++){
          const double h_ij = fmod(view_direction_h + view_width/2.0 + bearing(S.lat_standpoint, S.lon_standpoint, (H.lat + 1 - i/double(m-1))*deg2rad, (H.lon + j/double(n-1))*deg2rad) + 1.5*M_PI, 2*M_PI) * pixels_per_rad_h;
          if(h_ij < 0 || h_ij > width) continue;
          const double h_ijj = fmod(view_direction_h + view_width/2.0 + bearing(S.lat_standpoint, S.lon_standpoint, (H.lat + 1 - i/double(m-1))*deg2rad, (H.lon + (j+inc)/double(n-1))*deg2rad) + 1.5*M_PI, 2*M_PI) * pixels_per_rad_h;
          if(h_ijj < 0 || h_ijj > width) continue;
          const double h_iij = fmod(view_direction_h + view_width/2.0 + bearing(S.lat_standpoint, S.lon_standpoint, (H.lat + 1 - (i+inc)/double(m-1))*deg2rad, (H.lon + j/double(n-1))*deg2rad) + 1.5*M_PI ,2*M_PI) * pixels_per_rad_h;
          if(h_iij < 0 || h_iij > width) continue;
          const double h_iijj = fmod(view_direction_h + view_width/2.0 + bearing(S.lat_standpoint, S.lon_standpoint, (H.lat + 1 - (i+inc)/double(m-1))*deg2rad, (H.lon + (j+inc)/double(n-1))*deg2rad) + 1.5*M_PI, 2.0*M_PI) * pixels_per_rad_h;
          if(h_iijj < 0 || h_iijj > width) continue;

          //cout << S.z_standpoint << ", " << H(i,j) << ", " <<  D(i,j) << endl;
          const double v_ij   = (view_height/2.0 + view_direction_v - angle_v(S.z_standpoint, H(i,j), D(i,j))) * pixels_per_rad_v; // [px]
          if(v_ij < 0 || v_ij > height) continue;
          const double v_ijj  = (view_height/2.0 + view_direction_v - angle_v(S.z_standpoint, H(i,j+inc), D(i,j+inc))) * pixels_per_rad_v; //[px]
          if(v_ijj < 0 || v_ijj > height) continue;
          const double v_iij  = (view_height/2.0 + view_direction_v - angle_v(S.z_standpoint, H(i+inc,j), D(i+inc,j))) * pixels_per_rad_v; // [px]
          if(v_iij < 0 || v_iij > height) continue;
          const double v_iijj = (view_height/2.0 + view_direction_v - angle_v(S.z_standpoint, H(i+inc,j+inc), D(i+inc,j+inc))) * pixels_per_rad_v; // [px]
          if(v_iijj < 0 || v_iijj > height) continue;
          //debug << "v: " << v_ij << ", " << v_ijj << ", " << v_iij << ", " << v_iijj << endl;

          const double dist1 = (D(i,j)+D(i+inc,j)+D(i,j+inc))/3.0 - 2;
          const double dist2 = (D(i+inc,j)+D(i,j+inc)+D(i+inc,j+inc))/3.0 - 2;
          if(would_write_triangle(h_ij, v_ij, h_ijj, v_ijj, h_iij, v_iij, dist1)) peak_visible=true;
          if(would_write_triangle(h_ijj, v_ijj, h_iij, v_iij, h_iijj, v_iijj, dist2)) peak_visible=true;
#ifdef GRAPHICS_DEBUG
          write_triangle(h_ij, v_ij, h_ijj, v_ijj, h_iij, v_iij, dist1, 5*pow(dist1, 1.0/3.0), H(i,j)*(255.0/3500), 50);
          write_triangle(h_ijj, v_ijj, h_iij, v_iij, h_iijj, v_iijj, dist2, 5*pow(dist1, 1.0/3.0), H(i,j)*(255.0/3500) , 250);
#endif
        }
      }
      if(peak_visible){
        cout << peaks[p].name << " is visible" << endl;
        cout << "pixel will be written at : " << x_peak << ", " << y_peak << endl;
        write_pixel(x_peak, y_peak, 255,0,0);

        const int x_offset=0, y_offset=100;

        const int black = gdImageColorResolve(img_ptr, 0, 0, 0);
        gdImageLine(img_ptr, x_peak, y_peak-2, x_peak, y_peak-y_offset+5, black);

        string name(peaks[p].name);
        if(!peaks[p].name.empty()) name += ", ";
        name += to_string(peaks[p].elev) + "m, " + to_string(int(round(dist_peak/1000))) + "km";
        char *s = const_cast<char*>(name.c_str());
        const double fontsize = 12.;
        //char *font = "./palatino-59330a4da3d64.ttf";
        char *font = "./fonts/vera.ttf";
        const double text_orientation=M_PI/2;

        /* obtain bb so that we can size the image */
        int bb[8];
        char* err = gdImageStringFT(NULL,&bb[0],0,font,fontsize,0.,0,0,s);
        if (err) {fprintf(stderr,err); cout << "not good" << endl;}

//        cout << bb[0] << " " << bb[1] << " " << bb[2] << " " << bb[3] << " " << bb[4] << " " << bb[5] << " " << bb[6] << " " << bb[7] << endl;

        /* render the string, offset origin to center string*/
        /* note that we use top-left coordinate for adjustment
         * since gd origin is in top-left with y increasing downwards. */
        int xxx = 3 - bb[6];
        int yyy = 3 - bb[7];
        err = gdImageStringFT(img_ptr, &bb[0],
                              black, font, fontsize, text_orientation,
                              x_peak+fontsize/2.0,
                              y_peak-y_offset,s);
        if (err) {fprintf(stderr,err); cout << "not good" << endl;}

      }
#ifdef GRAPHICS_DEBUG
      else{
        cout << peaks[p].name << " is invisible" << endl;
        cout << "pixel will be written at : " << x_peak << ", " << y_peak << endl;
        write_pixel(x_peak, y_peak, 0,255,0);
      }
#endif

    }
  }
};

#endif

