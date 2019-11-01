#ifndef TILE_HH
#define TILE_HH

#include <cassert> // for assert
#include <cmath>   // for floor, abs, ceil
#include <cstdint> // for int16_t
#include <cstring>
#include <fstream> // for ostream, ifstream

#include "array2D.hh" // for array2D

using namespace std;

int16_t endian_swap(int16_t in);

// one tile only, without storing the viewpoint
template <typename T>
class tile: public array2D<T> {
  using array2D<T>::m;
  using array2D<T>::n;
  // [deg], specifying the lower left corner of the tile.  Hence, northern
  // tiles go from 0..89 while southern tiles go from 1..90, east: 0..179,
  // west: 1..180.  however, the array stores everything starting from the
  // top/left corner, row major.
  int lat, lon;
  int dim; // I expect either 3601 or 1201

public:
  tile(int _m, int _n, int _dim, int _lat, int _lon): array2D<T>(_m, _n), dim(_dim), lat(_lat), lon(_lon) { assert(this->m == this->n); }
  tile(array2D<T> A): array2D<T>(A) { assert(this->m == this->n); }
  tile(char const* FILENAME, int _dim, int _lat, int _lon): array2D<int16_t>(_dim, _dim), lat(_lat), lon(_lon), dim(_dim) {
    assert(dim > 0);
    // cout << " dimension in tile: " << dim << endl;
    const int size = dim * dim;
    int16_t size_test;

    ifstream ifs(FILENAME, ios::in | ios::binary);
    ifs.exceptions(ifstream::failbit | ifstream::badbit);
    try {
      ifs.read(reinterpret_cast<char*>(&((*this)(0, 0))), size * sizeof(size_test));
    }
    catch (const ifstream::failure& e) {
      cout << "Exception opening/reading file";
    }
    ifs.close();

    for (int i = 0; i < dim; i++) {
      for (int j = 0; j < dim; j++) {
        (*this)(i, j) = endian_swap((*this)(i, j));
      }
    }
  }

  int get_lat() const { return lat; }
  int get_lon() const { return lon; }
  int get_dim() const { return dim; }

  // viewfinder uses drop/m = 0.1695 m * (dist / miles)^2 to account for curvature and refraction
  tile<double> curvature_adjusted_elevations(const tile<double>& dists) const {
    // assert(this->m == dists.m);
    // assert(this->n == dists.n);
    const double coeff = 0.065444 / 1000000.0; // = 0.1695 / 1.609^2  // m
    tile<double> A(m, n, dim, lat, lon);
    for (int i = 0; i < m; i++) {
      for (int j = 0; j < n; j++) {
        A(i, j) = (*this)(i, j) - coeff * pow(dists(i, j), 2);
        //cout << (*this)(i,j) << ", " << dists(i,j) << " --> " << A(i,j) << endl;
      }
    }
    return A;
  }

  // matrix of distances [m] from standpoint to tile
  tile<double> get_distances(const double lat_standpoint, const double lon_standpoint) const {
    tile<double> A(m, n, dim, lat, lon);
    for (int i = 0; i < m; i++) {
      for (int j = 0; j < n; j++) {
        A(i, j) = distance_atan(lat_standpoint, lon_standpoint, (lat + 1 - i / double(m - 1)) * deg2rad, (lon + j / double(n - 1)) * deg2rad);
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
  double interpolate(const double lat_p, const double lon_p) const;

  friend ostream& operator<<(ostream& S, const tile& TT) {
    S << TT.n;
    for (int i = 0; i < TT.m; i++)
      S << " " << i + 1;
    S << endl;
    for (int i = 0; i < TT.m; i++) {
      S << TT.m - i << " ";
      for (int j = 0; j < TT.n; j++) {
        S << TT(i, j) << " ";
      }
      S << endl;
    }
    return S;
  }
};

#endif // TILE_HH
