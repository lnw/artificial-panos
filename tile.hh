#pragma once

#include <bit>
#include <cassert>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <fstream>

#include "array2d.hh"
#include "latlon.hh"


// one tile only, without storing the viewpoint
template <typename T>
class tile: public array2D<T> {
public:
  using array2D<T>::xs;
  using array2D<T>::ys;
  tile(int64_t _xs, int64_t _ys, int64_t _dim, int64_t _lat, int64_t _lon): array2D<T>(_xs, _ys), dim_(_dim), lat_(_lat), lon_(_lon) {
    assert(ys() == xs());
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
      ifs.read(std::bit_cast<char*>(array2D<T>::data().data()), size * sizeof(size_test));
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
    assert(ys() == dists.ys());
    assert(xs() == dists.xs());
    const double coeff = 0.065444 / 1000000.0; // = 0.1695 / 1.609^2  // m
    tile<double> A(xs(), ys(), dim_, lat_, lon_);
    for (int64_t y = 0; y < ys(); y++) {
      for (int64_t x = 0; x < xs(); x++) {
        A[x, y] = (*this)[x, y] - coeff * std::pow(dists[x, y], 2);
        // std::cout << (*this)(y,x) << ", " << dists(y,x) << " --> " << A(y,x) << std::endl;
      }
    }
    return A;
  }


  // matrix of distances [m] from standpoint to tile
  auto get_distances(const LatLon<double, Unit::rad> standpoint) const {
    std::vector<double> latitudes(ys()), longitudes(ys());
    for (int64_t y = 0; y < ys(); y++)
      latitudes[y] = (lat_ + 1 - y / double(ys() - 1));
    for (int64_t x = 0; x < xs(); x++)
      longitudes[x] = (lon_ + x / double(xs() - 1));

    tile<double> A(ys(), xs(), dim_, lat_, lon_);
#pragma omp parallel for
    for (int64_t y = 0; y < ys(); y++) {
      for (int64_t x = 0; x < xs(); x++) {
        const LatLon<double, Unit::deg> p(latitudes[y], longitudes[x]);
        A[x, y] = distance_atan(standpoint, p.to_rad());
        // A[x, y] = distance_acos(lat_standpoint, lon_standpoint, latitudes[y], longitudes[x]); // worse + slower
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
  constexpr double interpolate(LatLon<double, Unit::deg> coord) const {
    const auto [lat_p, lon_p] = coord;
    // std::cout << lat_p <<", "<< lon_p <<", "<<floor(lat_p) << ", "<< lat_ <<", " << floor(lon_p) <<", "<< lon_ << std::endl;
    assert(std::floor(lat_p) == lat_ && std::floor(lon_p) == lon_); // ie, we are in the right tile
    int64_t dim_m1 = dim_ - 1;                                      // we really need dim_-1 all the time
    // get the surrounding four indices
    int64_t y = dim_m1 - std::floor((lat_p - lat_) * dim_m1);
    int64_t yy = y - 1;
    int64_t x = std::floor((lon_p - lon_) * dim_m1);
    int64_t xx = x + 1;
    double lon_frac = dim_m1 * (lon_p - lon_) - x;
    double lat_frac = dim_m1 * (lat_p - lat_) - (dim_m1 - y);
    double aux1_h = (*this)[x, y] * (1 - lon_frac) + (*this)[xx, y] * lon_frac;
    // std::cout << "aux1_h: " << aux1_h << std::endl;
    double aux2_h = (*this)[x, yy] * (1 - lon_frac) + (*this)[xx, yy] * lon_frac;
    // std::cout << "aux2_h: " << aux2_h << std::endl;
    double p_h = aux1_h * (1 - lat_frac) + aux2_h * lat_frac;
    return p_h;
  }


  friend std::ostream& operator<<(std::ostream& S, const tile& TT) {
    S << TT.xs;
    for (int64_t y = 0; y < TT.ys; y++)
      S << " " << y + 1;
    S << std::endl;
    for (int64_t y = 0; y < TT.ys; y++) {
      S << TT.ys - y << " ";
      for (int64_t x = 0; x < TT.xs; x++) {
        S << TT[x, y] << " ";
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
