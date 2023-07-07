#pragma once

#include <bit>
#include <cassert>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <fstream>

#include "array2d.hh"


// one tile only, without storing the viewpoint
template <typename T>
class tile: public array2D<T> {
public:
  tile(int64_t _m, int64_t _n, int64_t _dim, int64_t _lat, int64_t _lon): array2D<T>(_m, _n), dim_(_dim), lat_(_lat), lon_(_lon) {
    assert(this->m() == this->n());
  }
  tile(array2D<T> A): array2D<T>(std::move(A)) {
    assert(this->m() == this->n());
  }
  tile(char const* FILENAME, int64_t _dim, int64_t _lat, int64_t _lon): array2D<int16_t>(_dim, _dim), dim_(_dim), lat_(_lat), lon_(_lon) {
    // auto t0 = std::chrono::high_resolution_clock::now();

    assert(dim_ > 0);
    // std::cout << " dimension in tile: " << dim_ << std::endl;
    const int64_t size = dim_ * dim_;

    std::ifstream ifs(FILENAME, std::ios::in | std::ios::binary);
    ifs.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    try {
      int16_t size_test;
      ifs.read(reinterpret_cast<char*>(&((*this)(0, 0))), size * sizeof(size_test));
    }
    catch (const std::ifstream::failure& e) {
      std::cout << "Exception opening/reading file";
    }
    ifs.close();

    // assuming the file is big endian:
    if (std::endian::native == std::endian::little)
      for (auto& v : *this)
        v = std::byteswap(v);

    // auto t1 = std::chrono::high_resolution_clock::now();
    // std::chrono::duration<double, std::milli> fp_ms = t1 - t0;
    // std::cout << "reading tile took " << fp_ms.count() << " ms" << std::endl;
  }

  constexpr auto lat() const { return lat_; }
  constexpr auto lon() const { return lon_; }
  constexpr auto dim() const { return dim_; }

  // viewfinder uses drop/m = 0.1695 m * (dist / miles)^2 to account for curvature and refraction
  tile<double> curvature_adjusted_elevations(const tile<double>& dists) const {
    assert(this->m() == dists.m());
    assert(this->n() == dists.n());
    const double coeff = 0.065444 / 1000000.0; // = 0.1695 / 1.609^2  // m
    tile<double> A(this->m(), this->n(), dim_, lat_, lon_);
    for (int64_t i = 0; i < this->m(); i++) {
      for (int64_t j = 0; j < this->n(); j++) {
        A(i, j) = (*this)(i, j) - coeff * std::pow(dists(i, j), 2);
        // std::cout << (*this)(i,j) << ", " << dists(i,j) << " --> " << A(i,j) << std::endl;
      }
    }
    return A;
  }


  // matrix of distances [m] from standpoint to tile
  auto get_distances(const double lat_standpoint, const double lon_standpoint) const {
    std::vector<double> latitudes(this->m()), longitudes(this->m());
    for (int64_t i = 0; i < this->m(); i++)
      latitudes[i] = (lat_ + 1 - i / double(this->m() - 1)) * deg2rad;
    for (int64_t j = 0; j < this->n(); j++)
      longitudes[j] = (lon_ + j / double(this->n() - 1)) * deg2rad;

    tile<double> A(this->m(), this->n(), dim_, lat_, lon_);
#pragma omp parallel for
    for (int64_t i = 0; i < this->m(); i++) {
      for (int64_t j = 0; j < this->n(); j++) {
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
    // std::cout << lat_p <<", "<< lon_p <<", "<<floor(lat_p) << ", "<< lat_ <<", " << floor(lon_p) <<", "<< lon_ << std::endl;
    assert(std::floor(lat_p) == lat_ && std::floor(lon_p) == lon_); // ie, we are in the right tile
    int64_t dim_m1 = dim_ - 1;                                      // we really need dim_-1 all the time
    // get the surrounding four indices
    int64_t i = dim_m1 - std::floor((lat_p - lat_) * dim_m1),
            ii = i - 1,
            j = std::floor((lon_p - lon_) * dim_m1),
            jj = j + 1;
    double lon_frac = dim_m1 * (lon_p - lon_) - j;
    double lat_frac = dim_m1 * (lat_p - lat_) - (dim_m1 - i);
    double aux1_h = (*this)(i, j) * (1 - lon_frac) + (*this)(i, jj) * lon_frac;
    // std::cout << "aux1_h: " << aux1_h << std::endl;
    double aux2_h = (*this)(ii, j) * (1 - lon_frac) + (*this)(ii, jj) * lon_frac;
    // std::cout << "aux2_h: " << aux2_h << std::endl;
    double p_h = aux1_h * (1 - lat_frac) + aux2_h * lat_frac;
    return p_h;
  }


  friend std::ostream& operator<<(std::ostream& S, const tile& TT) {
    S << TT.n;
    for (int64_t i = 0; i < TT.m; i++)
      S << " " << i + 1;
    S << std::endl;
    for (int64_t i = 0; i < TT.m; i++) {
      S << TT.m - i << " ";
      for (int64_t j = 0; j < TT.n; j++) {
        S << TT(i, j) << " ";
      }
      S << std::endl;
    }
    return S;
  }

private:
  // [deg], specifying the lower left corner of the tile.  Hence, northern
  // tiles go from 0..89 while southern tiles go from 1..90, east: 0..179,
  // west: 1..180.  however, the array stores everything starting from the
  // top/left corner, row major.
  int64_t dim_; // we expect either 3601 (1'') or 1201 (3'')
  int64_t lat_;
  int64_t lon_;
};
