#ifndef CANVAS_HH
#define CANVAS_HH

#include <vector>
#include <iostream>
#include <math.h>
#include <algorithm> // min, max


#include <png.h>

#include "array2D.hh"
#include "scene.hh"

using namespace std;


class canvas {

public:
  int width, height;
  array2D<double> zbuffer;
private:
  FILE *fp = nullptr;
  png_structp png_ptr = nullptr;
  png_infop info_ptr = nullptr;
  png_bytep *row_pointers = nullptr;

public:
  canvas(char const * filename, int x, int y): width(x), height(y), zbuffer(x,y,10000){
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

    for(size_t y = 0; y < height; y++) free(row_pointers[y]);
    free(row_pointers);

    png_destroy_write_struct(&png_ptr, &info_ptr);
    fclose(fp);
  }

  void write_pixel_zb(int x, int y, double z,
                      int16_t r, int16_t g, int16_t b, int16_t a){
    if (z < zbuffer(x,y)){
      zbuffer(x,y) = z;
      row_pointers[y][4*x]   = r;
      row_pointers[y][4*x+1] = g;
      row_pointers[y][4*x+2] = b;
      row_pointers[y][4*x+3] = a;
    }
  }

  void write_triangle(double x1, double y1, 
                      double x2, double y2, 
                      double x3, double y3, 
                      double z,
                      int16_t r, int16_t g, int16_t b, int16_t a){
    // find triangle's bb
    const int xmin = min( {floor(x1), floor(x2), floor(x3)} );
    const int xmax = max( {ceil(x1),  ceil(x2),  ceil(x3)} );
    const int ymin = min( {floor(y1), floor(y2), floor(y3)} );
    const int ymax = max( {ceil(y1),  ceil(y2),  ceil(y3)} );

    // iterate over grid points in bb, draw the ones in the triangle
    for (int i=xmin; i<xmax; i++){
      for (int j=ymin; j<ymax; j++){
        //if(point_in_triangle_1 (i+0.5,j+0.5,xmin-1,ymin-1, x1,y1,x2,y2,x3,y3)){
        if(point_in_triangle_2 (i+0.5,j+0.5, x1,y1,x2,y2,x3,y3)){
          write_pixel_zb(i,j,z, r,g,b,a);
        }
      }
    }
  }

  void render_test(){
    for (size_t y=0; y<height; y++) {
      png_byte* row = row_pointers[y];
      for (size_t x=0; x<width; x++) {
//        cout << x << endl;
        png_byte* ptr = &(row[x*4]);
//        printf("Pixel at position [ %d - %d ] has RGBA values: %d - %d - %d - %d\n",
//               x, y, ptr[0], ptr[1], ptr[2], ptr[3]);

        /* set red value to 0 and green value to the blue one */
        ptr[0] = x;
        ptr[1] = 0.1*x;
        ptr[2] = y;
        ptr[3] = 255; // 255 -> opaque, 0 -> transparent
      }
    }
  }

};

#endif

