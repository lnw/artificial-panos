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
  array2D(int64_t m, int64_t n, const std::vector<T>& A): n_(n), m_(m), dat_(A.begin(), A.end()) { assert(A.size() == m * n); }
  array2D(int64_t m, int64_t n, T init = 0): n_(n), m_(m), dat_(m * n, init) {}

  template <typename S>
  array2D(const array2D<S>& A): n_(A.n()), m_(A.m()), dat_(A.begin(), A.end()) {}

  constexpr T& operator[](int64_t n) { return dat_[n]; }
  constexpr T operator[](int64_t n) const { return dat_[n]; }
  constexpr T& operator[](int64_t i, int64_t j) { return dat_[i * n_ + j]; }
  constexpr T operator[](int64_t i, int64_t j) const { return dat_[i * n_ + j]; }

  constexpr auto n() const { return n_; }
  constexpr auto m() const { return m_; }

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
    std::swap(m_, n_);
    array2D<T> A(m_, n_);
    for (int64_t i = 0; i < m_; i++) {
      for (int64_t j = 0; j < n_; j++) {
        A[j, i] = (*this)[i, j];
      }
    }
    std::swap(*this, A);
  }

  friend std::ostream& operator<<(std::ostream& S, const array2D& A) {
    std::vector<std::vector<T>> VV(A.m(), std::vector<T>(A.n()));
    for (int64_t i = 0; i < A.m(); i++)
      for (int64_t j = 0; j < A.n(); j++)
        VV[i][j] = A[i * A.n() + j];

    S << VV;
    return S;
  }

protected:
  // n: number of columns (j->n)  // x
  // m: number of rows (i->m)  // y
  int64_t n_;
  int64_t m_;
  std::vector<T> dat_;
};
