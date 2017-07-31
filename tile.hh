#ifndef TILE_HH
#define TILE_HH

#include <assert.h>    // for assert
#include <stdint.h>    // for int16_t
#include <string.h>
#include <cmath>       // for floor, abs, ceil
#include <fstream>     // for ostream, ifstream

#include "array2D.hh"  // for array2D

using namespace std;

int16_t endian_swap(int16_t in){
  unsigned char c[2];
  memcpy(c, &in, 2);
  return (int16_t)(c[0] << 8 | c[1]);
}

// one tile only, without storing the viewpoint
template <typename T> class tile: public array2D<T> {

public:
  // [deg], specifying the lower left corner of the tile.  Hence, northern tiles go from 0..89 while southern tiles go from 1..90, east: 0..179, west: 1..180.
  // however, the array stores everything starting from the top/left corner, row major.
  int lat, lon;
  int dim; // I expect either 3601 or 1201

  tile(int _m, int _n, int _dim, int _lat, int _lon): array2D<T>(_m,_n), dim(_dim), lat(_lat), lon(_lon) {assert(this->m == this->n);}
  tile(array2D<T> A): array2D<T>(A) {assert(this->m == this->n);}
  tile(char const * FILENAME, int _dim, int _lat, int _lon): array2D<int16_t>(_dim,_dim), lat(_lat), lon(_lon), dim(_dim) {
    assert(dim > 0);
    // cout << " dimension in tile: " << dim << endl;
    const int size = dim*dim;
    int16_t size_test;

    ifstream ifs(FILENAME, ios::in | ios::binary);
    ifs.exceptions( ifstream::failbit | ifstream::badbit );
    try {
      ifs.read(reinterpret_cast<char *>(&((*this)(0,0))), size * sizeof(size_test));
    }
    catch (const ifstream::failure& e) {
      cout << "Exception opening/reading file";
    }
    ifs.close();

    for (int i=0; i<dim; i++){
      for (int j=0; j<dim; j++){
        (*this)(i,j) = endian_swap((*this)(i,j));
      }
    }
  }

  // viewfinder uses drop/m = 0.1695 m * (dist / miles)^2 to account for curvature and refraction
  tile<double> curvature_adjusted_elevations(const tile<double>& dists) const {
    // assert(this->m == dists.m);
    // assert(this->n == dists.n);
    const int& _m = this->m;
    const int& _n = this->n;
    tile<double> A(_m,_n,this->dim,lat,lon);
    for (int i=0; i<_m; i++){
      for (int j=0; j<_m; j++){
        const double coeff = 0.065444 / 1000000.0; // = 0.1695 / 1.609^2  // m
        A(i,j) = (*this)(i,j) - coeff*pow(dists(i,j),2);
  //cout << (*this)(i,j) << ", " << dists(i,j) << " --> " << A(i,j) << endl;
      }
    }
    return A;
  }

  // matrix of distances [m] from standpoint to tile
  tile<double> get_distances(const double lat_standpoint, const double lon_standpoint) const {
    const int& _m = this->m;
    const int& _n = this->n;
    tile<double> A(_m,_n,this->dim,lat,lon);
    for (int i=0; i<_m; i++){
      for (int j=0; j<_n; j++){
        A(i,j) = distance_atan(lat_standpoint, lon_standpoint, (lat + 1 - i/double(_m-1))*deg2rad, (lon + j/double(_n-1))*deg2rad);
      }
    }
    return A;
  }

// get elevation at lat_p, lon_p, given the correct tile
// ij---aux1---ijj
//        |
//        p
//        |
// iij--aux2---iijj
  double interpolate(const double lat_p, const double lon_p) const {
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

  friend ostream& operator<<(ostream& S, const tile& TT) {
    S << TT.n;
    for (int i=0;i<TT.m;i++)
      S << " " << i+1;
    S << endl;
    for (int i=0;i<TT.m;i++){
      S << TT.m - i << " ";
      for (int j=0;j<TT.n;j++){
        S << TT(i,j) << " ";
      }
      S << endl;
    }
    return S;
  }

};

#endif // TILE_HH
