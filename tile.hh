#pragma once

#include <cassert>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <fstream>

#include "array2D.hh"


template <int nBytes>
constexpr void swapEndianness(const char* in, char* out) {
  for (int i = 0; i < nBytes; i++) {
    out[i] = in[nBytes - i - 1];
  }
}

template <typename T>
constexpr T endian_swap(const T& var) {
  T res(0);
  swapEndianness<sizeof(var)>(reinterpret_cast<char const*>(&var), reinterpret_cast<char*>(&res));
  return res;
}


// one tile only, without storing the viewpoint
template <typename T>
class tile: public array2D<T> {
  using array2D<T>::m;
  using array2D<T>::n;
  // [deg], specifying the lower left corner of the tile.  Hence, northern
  // tiles go from 0..89 while southern tiles go from 1..90, east: 0..179,
  // west: 1..180.  however, the array stores everything starting from the
  // top/left corner, row major.
  int dim; // I expect either 3601 or 1201
  int lat, lon;

public:
  tile(int _m, int _n, int _dim, int _lat, int _lon): array2D<T>(_m, _n), dim(_dim), lat(_lat), lon(_lon) {
    assert(this->m == this->n);
  }
  tile(array2D<T> A): array2D<T>(A) {
    assert(this->m == this->n);
  }
  tile(char const* FILENAME, int _dim, int _lat, int _lon): array2D<int16_t>(_dim, _dim), dim(_dim), lat(_lat), lon(_lon) {
    // auto t0 = std::chrono::high_resolution_clock::now();

    assert(dim > 0);
    // std::cout << " dimension in tile: " << dim << std::endl;
    const int size = dim * dim;
    int16_t size_test;

    std::ifstream ifs(FILENAME, std::ios::in | std::ios::binary);
    ifs.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    try {
      ifs.read(reinterpret_cast<char*>(&((*this)(0, 0))), size * sizeof(size_test));
    }
    catch (const std::ifstream::failure& e) {
      std::cout << "Exception opening/reading file";
    }
    ifs.close();

    for (size_t i = 0; i < (*this).size(); i++)
      (*this)[i] = endian_swap((*this)[i]);

    // auto t1 = std::chrono::high_resolution_clock::now();
    // std::chrono::duration<double, std::milli> fp_ms = t1 - t0;
    // std::cout << "reading tile took " << fp_ms.count() << " ms" << std::endl;
  }

  int get_lat() const { return lat; }
  int get_lon() const { return lon; }
  int get_dim() const { return dim; }
  int get_m() const { return m; }
  int get_n() const { return n; }

  // viewfinder uses drop/m = 0.1695 m * (dist / miles)^2 to account for curvature and refraction
  tile<double> curvature_adjusted_elevations(const tile<double>& dists) const {
    assert(this->m == dists.get_m());
    assert(this->n == dists.get_n());
    const double coeff = 0.065444 / 1000000.0; // = 0.1695 / 1.609^2  // m
    tile<double> A(m, n, dim, lat, lon);
    for (int i = 0; i < m; i++) {
      for (int j = 0; j < n; j++) {
        A(i, j) = (*this)(i, j) - coeff * pow(dists(i, j), 2);
        // std::cout << (*this)(i,j) << ", " << dists(i,j) << " --> " << A(i,j) << std::endl;
      }
    }
    return A;
  }


  // matrix of distances [m] from standpoint to tile
  auto get_distances(const double lat_standpoint, const double lon_standpoint) const {
    std::vector<double> latitudes(m), longitudes(m);
    for (int i = 0; i < m; i++)
      latitudes[i] = (lat + 1 - i / double(m - 1)) * deg2rad;
    for (int j = 0; j < n; j++)
      longitudes[j] = (lon + j / double(n - 1)) * deg2rad;

    tile<double> A(m, n, dim, lat, lon);
#pragma omp parallel for
    for (int i = 0; i < m; i++) {
      for (int j = 0; j < n; j++) {
        A(i, j) = distance_atan<double>(lat_standpoint, lon_standpoint, latitudes[i], longitudes[j]);
        // A(i, j) = distance_acos(lat_standpoint, lon_standpoint, latitudes[i], longitudes[j]); // worse + slower
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
  constexpr double interpolate(const double lat_p, const double lon_p) const {
    // std::cout << lat_p <<", "<< lon_p <<", "<<floor(lat_p) << ", "<< lat <<", " << floor(lon_p) <<", "<< lon << std::endl;
    assert(std::floor(lat_p) == lat && std::floor(lon_p) == lon); // ie, we are in the right tile
    int dim_m1 = dim - 1;                                         // we really need dim-1 all the time
    // get the surrounding four indices
    int i = dim_m1 - std::floor((lat_p - lat) * dim_m1),
        ii = i - 1,
        j = std::floor((lon_p - lon) * dim_m1),
        jj = j + 1;
    double lon_frac = dim_m1 * (lon_p - lon) - j;
    double lat_frac = dim_m1 * (lat_p - lat) - (dim_m1 - i);
    double aux1_h = (*this)(i, j) * (1 - lon_frac) + (*this)(i, jj) * lon_frac;
    // std::cout << "aux1_h: " << aux1_h << std::endl;
    double aux2_h = (*this)(ii, j) * (1 - lon_frac) + (*this)(ii, jj) * lon_frac;
    // std::cout << "aux2_h: " << aux2_h << std::endl;
    double p_h = aux1_h * (1 - lat_frac) + aux2_h * lat_frac;
    return p_h;
  }


  friend std::ostream& operator<<(std::ostream& S, const tile& TT) {
    S << TT.n;
    for (int i = 0; i < TT.m; i++)
      S << " " << i + 1;
    S << std::endl;
    for (int i = 0; i < TT.m; i++) {
      S << TT.m - i << " ";
      for (int j = 0; j < TT.n; j++) {
        S << TT(i, j) << " ";
      }
      S << std::endl;
    }
    return S;
  }
};
