#ifndef CANVAS_HH
#define CANVAS_HH

#include <vector>
#include <iostream>
#include <math.h>
#include <algorithm> // min, max

#include <png.h>

#include "geometry.hh"
#include "array2D.hh"
#include "scene.hh"
#include "colour.hh"

using namespace std;


class canvas {

public:
  int width, height; // [pixels]
  array2D<double> zbuffer; // initialised to 1000 km [m]
private:
  FILE *fp = nullptr;
  png_structp png_ptr = nullptr;
  png_infop info_ptr = nullptr;
  png_bytep *row_pointers = nullptr;

public:
  canvas(char const * filename, int x, int y): width(x), height(y), zbuffer(x,y,1000000){
    fp = fopen(filename, "wb");
    if(!fp) abort();

    png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png_ptr) abort();

    info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr) abort();

    if (setjmp(png_jmpbuf(png_ptr))) abort();

    png_init_io(png_ptr, fp);

    // Output is 8bit depth, RGBA format.
    png_set_IHDR(
      png_ptr,
      info_ptr,
      width, height,
      8,
      PNG_COLOR_TYPE_RGBA,
      PNG_INTERLACE_NONE,
      PNG_COMPRESSION_TYPE_DEFAULT,
      PNG_FILTER_TYPE_DEFAULT
    );
    png_write_info(png_ptr, info_ptr);

    // png_bytep *row_pointers;
    row_pointers = (png_bytep*) malloc(sizeof(png_bytep) * height);
    for (size_t y=0; y<height; y++){
      row_pointers[y] = (png_byte*) malloc(png_get_rowbytes(png_ptr,info_ptr));
    }
  }

  ~canvas(){
    png_write_image(png_ptr, row_pointers);
    png_write_end(png_ptr, NULL); // or info_ptr

    for(size_t y=0; y<height; y++) free(row_pointers[y]);
    free(row_pointers);

    png_destroy_write_struct(&png_ptr, &info_ptr);
    fclose(fp);
  }

  void write_pixel_zb(const int x, const int y, const double z,
                      int16_t r, int16_t g, int16_t b, int16_t a){
    if (z < zbuffer(x,y)){
      zbuffer(x,y) = z;
      row_pointers[y][4*x]   = r;
      row_pointers[y][4*x+1] = g;
      row_pointers[y][4*x+2] = b;
      row_pointers[y][4*x+3] = a;
    }
  }

  void write_triangle(const double x1, const double y1, 
                      const double x2, const double y2, 
                      const double x3, const double y3, 
                      const double z,
                      int16_t r, int16_t g, int16_t b, int16_t a){
    // find triangle's bb
    const int xmin = min( {floor(x1), floor(x2), floor(x3)} );
    const int xmax = max( {ceil(x1),  ceil(x2),  ceil(x3)} );
    const int ymin = min( {floor(y1), floor(y2), floor(y3)} );
    const int ymax = max( {ceil(y1),  ceil(y2),  ceil(y3)} );

    // iterate over grid points in bb, draw the ones in the triangle
    for (size_t i=xmin; i<xmax; i++){
      for (size_t j=ymin; j<ymax; j++){
        if(point_in_triangle_2 (i+0.5,j+0.5, x1,y1,x2,y2,x3,y3)){
          write_pixel_zb(i,j,z, r,g,b,a);
        }
      }
    }
  }

  void render_scene(const scene& S){
    ofstream debug("debug-render_scene", ofstream::out | ofstream::app);
    // determine the dimensions, especially pixels/deg
    const double& view_direction = S.view_dir; // [rad]
    const double& view_width = S.view_width; // [rad]
    const double pixels_per_rad_h = width / view_width; // [px/rad]
    const double& view_height = S.view_height; // [rad]
    const double pixels_per_rad_v = height / view_height; // [px/rad]
    debug << "view direction [rad]: " << view_direction << endl;
    debug << "view width [rad]: " << view_width << endl;
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
      //for (size_t i=0; i<m-1; i++){
      //  for (size_t j=0; j<n-1; j++){
      const int inc = 1;
      for (size_t i=0; i<m-inc; i+=inc){
        for (size_t j=0; j<n-inc; j+=inc){
          if(D(i,j) > S.view_dist) continue;
          // first triangle: i/j, i+1/j, i/j+1
          // second triangle: i+1/j, i/j+1, i+1/j+1
          // get horizontal and vertical angles for all four points of the two triangles
          // translate to image coordinates
          const double h_ij = fmod(view_direction + view_width/2.0 + bearing(S.lat_standpoint, S.lon_standpoint, (H.lat + 1 - i/double(m-1))*deg2rad, (H.lon + j/double(n-1))*deg2rad) + 1.5*M_PI, 2*M_PI) * pixels_per_rad_h;
          if(h_ij < 0 || h_ij > width) continue;
          const double h_ijj = fmod(view_direction + view_width/2.0 + bearing(S.lat_standpoint, S.lon_standpoint, (H.lat + 1 - i/double(m-1))*deg2rad, (H.lon + (j+inc)/double(n-1))*deg2rad) + 1.5*M_PI, 2*M_PI) * pixels_per_rad_h;
          if(h_ijj < 0 || h_ijj > width) continue;
          const double h_iij = fmod(view_direction + view_width/2.0 + bearing(S.lat_standpoint, S.lon_standpoint, (H.lat + 1 - (i+inc)/double(m-1))*deg2rad, (H.lon + j/double(n-1))*deg2rad) + 1.5*M_PI ,2*M_PI) * pixels_per_rad_h;
          if(h_iij < 0 || h_iij > width) continue;
          const double h_iijj = fmod(view_direction + view_width/2.0 + bearing(S.lat_standpoint, S.lon_standpoint, (H.lat + 1 - (i+inc)/double(m-1))*deg2rad, (H.lon + (j+inc)/double(n-1))*deg2rad) + 1.5*M_PI, 2.0*M_PI) * pixels_per_rad_h;
          if(h_iijj < 0 || h_iijj > width) continue;
          //debug << "("<<i<<","<<j<< ") h: " << h_ij << ", " << h_ijj << ", " << h_iij << ", " << h_iijj << endl;

          // there should be a check here to avoid triangles to wrap around
         
          //cout << S.z_standpoint << ", " << H(i,j) << ", " <<  D(i,j) << endl;
          const double v_ij   = (view_height/2.0 - angle_v(S.z_standpoint, H(i,j), D(i,j))) * pixels_per_rad_v; // [px]
          if(v_ij < 0 || v_ij > height) continue;
          const double v_ijj  = (view_height/2.0 - angle_v(S.z_standpoint, H(i,j+inc), D(i,j+inc))) * pixels_per_rad_v; //[px]
          if(v_ijj < 0 || v_ijj > height) continue;
          const double v_iij  = (view_height/2.0 - angle_v(S.z_standpoint, H(i+inc,j), D(i+inc,j))) * pixels_per_rad_v; // [px]
          if(v_iij < 0 || v_iij > height) continue;
          const double v_iijj = (view_height/2.0 - angle_v(S.z_standpoint, H(i+inc,j+inc), D(i+inc,j+inc))) * pixels_per_rad_v; // [px]
          if(v_iijj < 0 || v_iijj > height) continue;
          //debug << "v: " << v_ij << ", " << v_ijj << ", " << v_iij << ", " << v_iijj << endl;
          //cout << v_ij << endl;
              
          const double dist1 = (D(i,j)+D(i+inc,j)+D(i,j+inc))/3.0;
          write_triangle(h_ij, v_ij, h_ijj, v_ijj, h_iij, v_iij, dist1, D(i,j)*(255.0/30000), H(i,j)*(255.0/700), 150, 255);
          const double dist2 = (D(i+inc,j)+D(i,j+inc)+D(i+inc,j+inc))/3.0;
          //cout << "d12: " << dist1/10 << ", " << dist2/10 << endl;
          write_triangle(h_ijj, v_ijj, h_iij, v_iij, h_iijj, v_iijj, dist2, D(i,j)*(255.0/30000), H(i,j)*(255.0/700) , 150, 255);
        }
      }
    }
    debug.close();
  }

  void render_test(){
    for (size_t y=0; y<height; y++) {
      png_byte* row = row_pointers[y];
      for (size_t x=0; x<width; x++) {
//        cout << x << endl;
        png_byte* ptr = &(row[x*4]);
//        printf("Pixel at position [ %d - %d ] has RGBA values: %d - %d - %d - %d\n",
//               x, y, ptr[0], ptr[1], ptr[2], ptr[3]);

        ptr[0] = x; // r
        ptr[1] = 0.1*x; // g
        ptr[2] = y; // b
        ptr[3] = 255; // 0 -> transparent, 255 -> opaque
      }
    }
  }

  void bucket_fill( const int r, const int g, const int b, const int a ){
    for (size_t y=0; y<height; y++) {
      png_byte* row = row_pointers[y];
      for (size_t x=0; x<width; x++) {
        png_byte* ptr = &(row[x*4]);
        ptr[0] = r; // r
        ptr[1] = g; // g
        ptr[2] = b; // b
        ptr[3] = a; // 0 -> transparent, 255 -> opaque
      }
    }
  }

};

#endif

