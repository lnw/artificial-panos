#pragma once

#include <bit>
#include <cassert>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <ranges>
#include <string>
#include <vector>

#include "array2d.hh"
#include "latlon.hh"

namespace fs = std::filesystem;

// one tile only, without storing the viewpoint
template <typename T>
class tile: public array2D<T> {
public:
  using array2D<T>::xs;
  using array2D<T>::ys;
  tile() = default;
  tile(int64_t _xs, int64_t _ys, int64_t _dim, LatLon<int64_t, Unit::deg> _coord): array2D<T>(_xs, _ys), dim_(_dim), coord_(_coord) {
    assert(ys() == xs());
  }

  tile(const fs::path& fn, int64_t _dim, LatLon<int64_t, Unit::deg> _coord): array2D<int16_t>(_dim, _dim), dim_(_dim), coord_(_coord) {
    // auto t0 = std::chrono::high_resolution_clock::now();

    assert(dim_ > 0);
    // std::cout << " dimension in tile: " << dim_ << std::endl;
    const int64_t size = dim_ * dim_;

    std::ifstream ifs(fn.string(), std::ios::in | std::ios::binary);
    ifs.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    try {
      ifs.read(std::bit_cast<char*>(array2D<T>::data().data()), size * sizeof(int16_t));
    }
    catch (const std::ifstream::failure& e) {
      std::cout << "Exception opening/reading file"
                << " " << fn.string();
    }
    ifs.close();

    // assuming the file is big endian:
    if (std::endian::native == std::endian::little) {
      for (auto& v : *this) {
        v = std::byteswap(v);
      }
    }

    // auto t1 = std::chrono::high_resolution_clock::now();
    // std::chrono::duration<double, std::milli> fp_ms = t1 - t0;
    // std::cout << "reading tile took " << fp_ms.count() << " ms" << std::endl;
  }

  constexpr auto lat() const noexcept { return coord_.lat(); }
  constexpr auto lon() const noexcept { return coord_.lon(); }
  constexpr auto coord() const noexcept { return coord_; }
  constexpr auto dim() const noexcept { return dim_; }

  // viewfinder uses drop/m = 0.1695 m * (dist / miles)^2 to account for curvature and refraction
  template <typename U>
  requires std::floating_point<U>
  auto curvature_adjusted_elevations(const tile<U>& dists) const {
    assert(ys() == dists.ys());
    assert(xs() == dists.xs());
    const U coeff = 0.065444 / 1000000.0; // = 0.1695 / 1.609^2  // m
    tile<U> A(xs(), ys(), dim(), coord());
    std::transform(this->begin(), this->end(), dists.begin(), A.begin(), [coeff](auto el, auto dist) { return el - coeff * dist * dist; });
    return A;
  }


  // matrix of distances [m] from standpoint to tile
  template <typename U>
  requires std::floating_point<U>
  auto get_distances(const LatLon<U, Unit::rad> standpoint) const {
    std::vector<U> longitudes(xs());
    std::vector<U> latitudes(ys());
    for (int64_t y = 0; y < ys(); y++)
      latitudes[y] = lat() + 1 - y / U(ys() - 1);
    for (int64_t x = 0; x < xs(); x++)
      longitudes[x] = lon() + x / U(xs() - 1);

    tile<U> A(ys(), xs(), dim(), coord());
#if 1
    for (int64_t y = 0; y < ys(); y++) {
      for (int64_t x = 0; x < xs(); x++) {
        const LatLon<U, Unit::deg> p(latitudes[y], longitudes[x]);
        A[x, y] = distance_atan(standpoint, p.to_rad());
        // A[x, y] = distance_acos(lat_standpoint, lon_standpoint, latitudes[y], longitudes[x]); // worse + slower
      }
    }
#else
    for (const auto [index, coord] : std::ranges::views::enumerate(std::ranges::views::cartesian_product(latitudes, longitudes))) {
      A[index] = distance_atan(standpoint, LatLon<U, Unit::deg>(coord).to_rad());
    }
#endif
    return A;
  }


  // get elevation at lat_p, lon_p, given the correct tile
  // ij---aux1---ijj
  //        |
  //        p
  //        |
  // iij--aux2---iijj
  template <typename U>
  constexpr auto interpolate(LatLon<U, Unit::deg> coord) const {
    const auto [lat_p, lon_p] = coord;
    // std::cout << lat_p <<", "<< lon_p <<", "<<floor(lat_p) << ", "<< lat() <<", " << floor(lon_p) <<", "<< lon_ << std::endl;
    assert(std::floor(lat_p) == lat() && std::floor(lon_p) == lon_); // ie, we are in the right tile
    int64_t dim_m1 = dim() - 1;                                      // we really need dim_-1 all the time
    // get the surrounding four indices
    int64_t y = dim_m1 - std::floor((lat_p - lat()) * dim_m1);
    int64_t yy = y - 1;
    int64_t x = std::floor((lon_p - lon()) * dim_m1);
    int64_t xx = x + 1;
    U lon_frac = dim_m1 * (lon_p - lon()) - x;
    U lat_frac = dim_m1 * (lat_p - lat()) - (dim_m1 - y);
    U aux1_h = (*this)[x, y] * (U(1) - lon_frac) + (*this)[xx, y] * lon_frac;
    // std::cout << "aux1_h: " << aux1_h << std::endl;
    U aux2_h = (*this)[x, yy] * (U(1) - lon_frac) + (*this)[xx, yy] * lon_frac;
    // std::cout << "aux2_h: " << aux2_h << std::endl;
    U p_h = aux1_h * (U(1) - lat_frac) + aux2_h * lat_frac;
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
  int64_t dim_; // we expect either 3601 (1'') or 1201 (3'')
  // [deg], specifying the lower left corner of the tile.  Hence, northern
  // tiles go from 0..89 while southern tiles go from 1..90, east: 0..179,
  // west: 1..180.  however, the array stores everything starting from the
  // top/left corner, row major.
  LatLon<int64_t, Unit::deg> coord_;
};
