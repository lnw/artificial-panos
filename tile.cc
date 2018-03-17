
#include <cassert>    // for assert
#include <cstdint>    // for int16_t
#include <cstring>
#include <cmath>       // for floor, abs, ceil
#include <fstream>     // for ostream, ifstream

#include "array2D.hh"  // for array2D
#include "tile.hh"

using namespace std;

int16_t endian_swap(int16_t in){
  unsigned char c[2];
  memcpy(c, &in, 2);
  return (int16_t)(c[0] << 8 | c[1]);
}


// get elevation at lat_p, lon_p, given the correct tile
// ij---aux1---ijj
//        |
//        p
//        |
// iij--aux2---iijj
// input in deg
template<>
double tile<double>::interpolate(const double lat_p, const double lon_p) const {
//    cout << lat_p <<", "<< lon_p <<", "<<floor(lat_p) << ", "<< lat <<", " << floor(lon_p) <<", "<< lon << endl;
  assert(floor(lat_p) == lat && floor(lon_p) == lon);
  const int dim_m1 = dim-1; // we really need dim-1 all the time
  // get the surrounding four indices
  const int i = dim_m1 - floor((lat_p - lat)*dim_m1),
           ii = dim_m1 - ceil((lat_p - lat)*dim_m1),
            j = floor((lon_p - lon)*dim_m1),
           jj = ceil((lon_p - lon)*dim_m1);

//    cout << "i,ii: " << i<< ", " <<ii << endl;
//    cout << "j,jj: " << j<< ", " <<jj << endl;

// cout << (*this)(i,j) << ", " <<  abs(dim_m1*lon_p - dim_m1*floor(lon_p)-jj)   << ", "<< (*this)(i,jj) << ", " << abs(dim_m1*lon_p - dim_m1*floor(lon_p) - j) << endl;
  const double aux1_h = (*this)(i,j) * abs(dim_m1*(lon_p-floor(lon_p))-jj) + (*this)(i,jj) * abs(dim_m1*(lon_p-floor(lon_p))-j);
// cout << "aux1_h: " << aux1_h << endl;
  const double aux2_h = (*this)(ii,j) * abs(dim_m1*(lon_p-floor(lon_p))-jj) + (*this)(ii,jj) * abs(dim_m1*(lon_p-floor(lon_p))-j);
// cout << "aux2_h: " << aux2_h << endl;
// cout << aux1_h <<", "<< abs(dim_m1*(lat_p-floor(lat_p))-(dim_m1-ii)) <<", "<< aux2_h <<", "<< abs(dim_m1*(lat_p-floor(lat_p))-(dim_m1-i)) << endl;
  const double p_h = aux1_h * abs(dim_m1*(lat_p-floor(lat_p))-(dim_m1-ii)) + aux2_h * abs(dim_m1*(lat_p-floor(lat_p))-(dim_m1-i));
  return p_h;
}

