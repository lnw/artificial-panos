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
  int lat, lon; // deg

  tile(array2D<T> A): array2D<T>(A) {assert(this->m==this->n);}

  void adjust_curvature(const double lat, const double lon){
    for (int i=0; i<this->m; i++){
      for (int j=0; j<this->n; j++){
        
      }
    }
  }

};

#endif
