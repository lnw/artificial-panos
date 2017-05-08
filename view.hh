#ifndef VIEW_HH
#define VIEW_HH

#include <vector>
#include <iostream>
#include <cmath>
// #include <climits>

#include "array2D.hh"
#include "scene.hh"

using namespace std;

class view {

public:
  int x, y;
  // some libpng thing
  array2D<double> zbuffer;

  view(int x, int y, scene S): x(x), y(y), zbuffer(x,y,10000) {};



};

#endif
