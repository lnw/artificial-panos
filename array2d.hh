#pragma once

#include <algorithm>
#include <array>
#include <cassert>
#include <vector>

#include "auxiliary.hh"
#include "geometry.hh"


template <typename T>
class array2D {
public:
  using value_type = T;
  array2D() = default;
  array2D(int64_t xs, int64_t ys, const std::vector<T>& A): size_{xs, ys}, dat_(A.begin(), A.end()) { assert(A.size() == ys * xs); }
  array2D(int64_t xs, int64_t ys, T init = 0): size_{xs, ys}, dat_(ys * xs, init) {}

  template <typename S>
  array2D(const array2D<S>& A): size_{A.xs(), A.ys()}, dat_(A.begin(), A.end()) {}

  constexpr T& operator[](int64_t n) noexcept { return dat_[n]; }
  constexpr T operator[](int64_t n) const noexcept { return dat_[n]; }
  constexpr T& operator[](int64_t x, int64_t y) noexcept { return dat_[y * xs() + x]; }
  constexpr T operator[](int64_t x, int64_t y) const noexcept { return dat_[y * xs() + x]; }

  constexpr auto xs() const { return size_[0]; }
  constexpr auto ys() const { return size_[1]; }
  constexpr auto size() const { return size_; }

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
    std::swap(size_[0], size_[1]);
    array2D<T> A(ys(), xs());
    for (int64_t y = 0; y < ys(); y++) {
      for (int64_t x = 0; x < xs(); x++) {
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
  std::array<int64_t, 2> size_;
  std::vector<T> dat_;
};
