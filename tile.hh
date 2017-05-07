#ifndef TILE_HH
#define TILE_HH

#include <vector>
#include <iostream>
//#include <climits>
//#include <cassert>
#include <cmath>

#include "array2D.hh"

using namespace std;

// one tile only, without storing the viewpoint
template <typename T> class tile: public array2D<T> {

public:
  int lat, lon; // deg // NE: lower-left corner

  tile(int m, int n): array2D<T>(m,n) {assert(this->m == this->n);}
  tile(array2D<T> A): array2D<T>(A) {assert(this->m == this->n);}

  // viewfinder uses drop/m = 0.1695 m * (dist / miles)^2 to account for curvature and refraction
  tile<double> adjust_curvature(tile<double> heights){
    assert(this->m == heights.m);
    assert(this->n == heights.n);
    const int& m = this->m;
    const int& n = this->n;
    tile<double> A(m,n);
    for (int i=0; i<m; i++){
      for (int j=0; j<n; j++){
        const double coeff = 0.065444; // = 0.1695 / 1.609^2  // m
        A(i,j) = (*this)(i,j) - coeff*pow(heights(i,j),2);
      }
    }
    return A;
  }

  tile<double> get_distances(const double lat_standpoint, const double lon_standpoint){
    const int& m = this->m;
    const int& n = this->n;
    tile<double> A(m,n);
    for (int i=0; i<m; i++){
      for (int j=0; j<n; j++){
        A(i,j) = distance_atan(lat_standpoint, lon_standpoint, lat + i/(m-1), lon + j/(n-1));
      }
    }
    return A;
  }


};

#endif
