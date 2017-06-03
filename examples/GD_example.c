/***
 * GD_example.c  - an example of creating an image with the GD library
 *
 * This code shows a short example of 
 *   (1) creating an indexed color image and allocating some colors in it,
 *   (2) drawing some lines, 
 *   (3) filling in some pixels, and
 *   (4) writing it to a file.
 * 
 * For documentation on GD see
 *   http://www.libgd.org          
 *   http://www.libgd.org/OldManual          (as of Feb 2 2007)
 *   http://www.libgd.org/OldImageCreation
 *   http://www.boutell.com/gd/manual2.0.33.html
 * However, I found the system header file to be all I really needed :
 *   /usr/include/gd.h  (on cs.marlboro.edu)
 *
 * Installing the GD library using Ubuntu's apt-get package manager
 * looks like this
 *   $ apt-get install libgd2-xpm-dev
 * or you could get it from its website and follow their installation docs.
 *
 * Compiling and running this file (on cs) looks like this.
 *   $ gcc -lgd GD_example.c -o GD_example
 *   $ ./GD_example
 *   === GD example ===
 *   Creating 300 by 300 image.
 *   Drawing some blue lines.
 *   Filling in some gray pixels.
 *   Creating output file 'GD_example.png'.
 * Note that the "-lgd" means "include the gd library".
 * Then of course you'd just use a web browser to look at GD_example.png.
 *
 * Other libraries which may be required (depending on what you're doing)
 * according to the GD docs include
 *     in gcc line     in C code                    what
#      -----------     --------------------         -----------------
 *      -lz             #include <zlib>              compression
 *      -ljpeg          #include <jpeglib>           jpeg utilties
 *      -lfreetype      #include <freetype>          font routines
 *      -lm             #include <math>              math functions
 * See the /usr/include/*lib.h or *.h header files for more information on 'em.
 *
 * Note that GD allows two different types of images: 
 *    indexed color (default), which contains a list of allocated colors, and
 *    true color    for which GD packs (r,g,b) colors into ints
 * and that here I'm using the indexed color one.
 *
 * Jim Mahoney, Feb 2 2007
 **********************************/
#include <stdio.h>
#include <gd.h>

// Dimensions of image in pixels
#define IMAGE_WIDTH  300
#define IMAGE_HEIGHT 300

// The data to display is in a DATA_SIZE x DATA_SIZE array,
// which will have its (left,top)=(x,y) corner at 
// (DATA_TOP, DATA_LEFT) pixels in from the (0,0)=(left,top) pixel.
#define DATA_SIZE    8
#define DATA_LEFT    30
#define DATA_TOP     30

// Position of some blue lines drawn in the image.
#define BORDER       10
#define LEFT         BORDER
#define RIGHT        IMAGE_WIDTH - BORDER
#define TOP          BORDER
#define BOTTOM       IMAGE_HEIGHT - BORDER

// See the bottom of this code for a discussion of some output possibilities.
char*   filename =   "GD_example.png";

// Some values that'll go into the image as shades of gray.
// Range is 0 to 255 (i.e. 8 bits, which is the standard range of intensity).
// To do this sort of thing with floating point data or other data,
// you'd first scale your numbers to be in this 0 to 255 range.
int     data[DATA_SIZE][DATA_SIZE] ={{  2,  10,  10,  10,  10,  10,  10,   2},
                                     { 10,  20,  30,  40,  40,  30,  20,  10},
                                     { 10,  30, 100, 100, 100, 100,  30,  10},
                                     { 10,  40, 100, 200, 200, 100,  40,  10},
                                     { 10,  30, 100, 200, 200, 100,  30,  10},
                                     { 10,  20, 100, 100, 100, 100,  20,  10},
                                     { 10,  10,  20,  30,  40,  30,  10,  10},
                                     {  2,  10,  10,  10,  10,  10,  10,   2}};
int main(){
  FILE*       outfile;                                  // defined in stdio
  gdImagePtr  image;                                    // a GD image object
  int         white, blue, gray[255];                   // some GD colors 
  int         i, x, y;                                  // array subscripts

  printf("=== GD example ===\n");

  printf("Creating %i by %i image.\n", IMAGE_WIDTH, IMAGE_HEIGHT);
  image = gdImageCreate(IMAGE_WIDTH, IMAGE_HEIGHT);
  // Or image = gdImageCreateTrueColor(IMAGE_WIDTH, IMAGE_HEIGHT);
  //    followed by colors like white=gdTrueColor(255,255,255) that don't
  //    need to refer to any one image's color table.
  white = gdImageColorAllocate(image, 255,255,255);    //  1st is background
  blue  = gdImageColorAllocate(image, 0,0,255);        //  (red,green,blue)
  for (i=0; i<255; i++){
    gray[i] = gdImageColorAllocate(image, i,i,i);
  }

  printf("Drawing some blue lines.\n");
  gdImageLine(image, LEFT,TOP,     RIGHT,TOP,    blue); // draw lines in image
  gdImageLine(image, RIGHT,TOP,    RIGHT,BOTTOM, blue); //  +-----------------+
  gdImageLine(image, RIGHT,BOTTOM, LEFT,BOTTOM,  blue); //  |0,0       WIDTH,0|
  gdImageLine(image, LEFT,BOTTOM,  LEFT,TOP,     blue); //  |0,HEIGHT         |
                                                        //  +-----------------+
  printf("Filling in some gray pixels.\n");
  for (x=0; x<DATA_SIZE; x++){                          // fill some grayscale
    for (y=0; y<DATA_SIZE; y++){                        // colors from data.
      gdImageSetPixel(image, x+DATA_LEFT, y+DATA_TOP, gray[data[x][y]]);    
    }
  }

  // Finally, write the image out to a file.
  printf("Creating output file '%s'.\n", filename);
  outfile = fopen(filename, "wb");
  gdImagePng(image, outfile);
  fclose(outfile);

  /**********
   * Notes about the output :
   *
   *  1. "wb" here means "write binary"; the 'binary' part only 
   *     applies to Windows but doesn't hurt on unix boxes.
   *
   *  2. If this had be a true color image then probably a JPEG would
   *     be a better choice than a PNG; in that case you'd just say
   *        gdImageJpeg(image, outfile, quality);
   *     where 0<=quality<=100, or quality=-1 for the libjpeg default.
   *
   *  3. To send the output image to standard output, you'd just say
   *        gdImagePng(image, stdout);   
   *      in which case you'd run the program like this :
   *        $ ./GD_example > outputfile.png
   *
   ************/

}
