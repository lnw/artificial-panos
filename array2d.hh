#pragma once

#include <algorithm>
#include <cassert>
#include <iostream>
#include <vector>

#include "auxiliary.hh"
#include "geometry.hh"


template <typename T>
class array2D {
public:
  array2D(int64_t xs, int64_t ys, const std::vector<T>& A): xs_(xs), ys_(ys), dat_(A.begin(), A.end()) { assert(A.size() == ys * xs); }
  array2D(int64_t xs, int64_t ys, T init = 0): xs_(xs), ys_(ys), dat_(ys * xs, init) {}

  template <typename S>
  array2D(const array2D<S>& A): xs_(A.xs()), ys_(A.ys()), dat_(A.begin(), A.end()) {}

  constexpr T& operator[](int64_t n) noexcept { return dat_[n]; }
  constexpr T operator[](int64_t n) const noexcept { return dat_[n]; }
  constexpr T& operator[](int64_t x, int64_t y) noexcept { return dat_[y * xs_ + x]; }
  constexpr T operator[](int64_t x, int64_t y) const noexcept { return dat_[y * xs_ + x]; }

  constexpr auto xs() const { return xs_; }
  constexpr auto ys() const { return ys_; }

  constexpr auto& data() const& { return dat_; }
  constexpr auto&& data() && { return dat_; }

  constexpr auto begin() { return dat_.begin(); }
  constexpr auto begin() const { return dat_.cbegin(); }
  constexpr auto end() { return dat_.end(); }
  constexpr auto end() const { return dat_.cend(); }

  array2D operator+(const array2D& y) const { return array2D(*this) += y; }
  array2D& operator+=(const array2D& y) {
    std::transform(begin(), end(), y.begin(), begin(), [](auto v1, auto v2) { return v1 + v2; });
    return *this;
  }
  array2D operator-(const array2D& y) const { return array2D(*this) -= y; }
  array2D& operator-=(const array2D& y) {
    std::transform(begin(), end(), y.begin(), begin(), [](auto v1, auto v2) { return v1 - v2; });
    return *this;
  }

  array2D pointwise_min(const array2D& B) const {
    array2D Tnew(*this);
    std::transform(begin(), end(), Tnew.begin(), Tnew.begin(), [](auto v1, auto v2) { return std::min(v1, v2); });
    return Tnew;
  }

  constexpr void transpose() {
    std::swap(ys_, xs_);
    array2D<T> A(ys_, xs_);
    for (int64_t y = 0; y < ys_; y++) {
      for (int64_t x = 0; x < xs_; x++) {
        A[x, y] = (*this)[y, x];
      }
    }
    std::swap(*this, A);
  }

  friend std::ostream& operator<<(std::ostream& S, const array2D& A) {
    std::vector<std::vector<T>> VV(A.ys(), std::vector<T>(A.xs()));
    for (int64_t y = 0; y < A.ys(); y++)
      for (int64_t x = 0; x < A.xs(); x++)
        VV[y][x] = A[x, y];

    S << VV;
    return S;
  }

protected:
  // n: number of columns (j->n)  // x
  // m: number of rows (i->m)  // y
  int64_t xs_;
  int64_t ys_;
  std::vector<T> dat_;
};