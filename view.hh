#ifndef VIEW_HH
#define VIEW_HH

#include <vector>
#include <iostream>
#include <cmath>
// #include <climits>

#include <png.h>

#include "array2D.hh"
#include "scene.hh"

using namespace std;

class view {

public:
  int width, height;
  // some libpng thing
  array2D<double> zbuffer;

  view(int x, int y, scene S): width(x), height(y), zbuffer(x,y,10000) {};

  void render(){
png_bytep *row_pointers;
    char const * filename = "out.png";
    FILE *fp = fopen(filename, "wb");
    if(!fp) abort();

    png_structp png = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png) abort();

    png_infop info = png_create_info_struct(png);
    if (!info) abort();

    if (setjmp(png_jmpbuf(png))) abort();

    png_init_io(png, fp);

    // Output is 8bit depth, RGBA format.
    png_set_IHDR(
      png,
      info,
      width, height,
      8,
      PNG_COLOR_TYPE_RGBA,
      PNG_INTERLACE_NONE,
      PNG_COMPRESSION_TYPE_DEFAULT,
      PNG_FILTER_TYPE_DEFAULT
    );
    png_write_info(png, info);

    // To remove the alpha channel for PNG_COLOR_TYPE_RGB format,
    // Use png_set_filler().
    //png_set_filler(png, 0, PNG_FILLER_AFTER);

//    png_write_image(png, row_pointers);
    png_write_end(png, NULL);

//    for(int y = 0; y < height; y++) {
//      free(row_pointers[y]);
//    }
//    free(row_pointers);

    fclose(fp);
  }

};

#endif
