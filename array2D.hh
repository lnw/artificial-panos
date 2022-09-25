#pragma once

#include <cassert>
#include <iostream>
#include <vector>

#include "auxiliary.hh"
#include "geometry.hh"


template <typename T>
class array2D: public std::vector<T> {
protected:
  // m: number of rows (i->m)  // y
  // n: number of columns (j->n)  // x
  int m, n;

public:
  array2D(int _m, int _n, const std::vector<T>& A): std::vector<T>(A.begin(), A.end()), m(_m), n(_n) { assert(A.size() == _m * _n); }
  array2D(int _m, int _n, const T& zero = 0): std::vector<T>(_m * _n, zero), m(_m), n(_n) {}

  template <typename S>
  array2D(const array2D<S>& A): std::vector<T>(A.begin(), A.end()), m(A.get_m()), n(A.get_n()) {}

  constexpr T& operator()(int i, int j) { return (*this)[i * n + j]; }
  constexpr T operator()(int i, int j) const { return (*this)[i * n + j]; }

  array2D operator+(const array2D& y) const { return array2D(*this) += y; }
  array2D& operator+=(const array2D& y) {
    for (int i = 0; i < n * m; i++)
      (*this)[i] += y[i];
    return *this;
  }
  array2D operator-(const array2D& y) const { return array2D(*this) -= y; }
  array2D& operator-=(const array2D& y) {
    for (int i = 0; i < n * m; i++)
      (*this)[i] -= y[i];
    return *this;
  }

  array2D pointwise_min(const array2D& B) const {
    array2D Tnew(*this);
    for (int i = 0; i < n * m; i++)
      Tnew[i] = std::min(Tnew[i], B[i]);
    return Tnew;
  }

  constexpr int get_m() const { return m; }
  constexpr int get_n() const { return n; }

  constexpr void transpose() {
    int x(m);
    m = n;
    n = x;
    array2D<T> A(m, n);
    for (int i = 0; i < m; i++) {
      for (int j = 0; j < n; j++) {
        A(j, i) = (*this)(i, j);
      }
    }
    std::swap(*this, A);
  }

  friend std::ostream& operator<<(std::ostream& S, const array2D& A) {
    std::vector<std::vector<T>> VV(A.m, std::vector<T>(A.n));
    for (int i = 0; i < A.m; i++)
      for (int j = 0; j < A.n; j++)
        VV[i][j] = A[i * A.n + j];

    S << VV;
    return S;
  }
};
