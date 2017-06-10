#ifndef LABELGROUP_HH
#define LABELGROUP_HH

#include <vector>
#include <tuple>
#include <iostream>
#include <math.h>
#include <algorithm> // min, max
// #include <cmath> // modf


//#include "geometry.hh"
//#include "array2D.hh"
//#include "tile.hh"
#include "mapitems.hh"

using namespace std;


class LabelGroup{
  vector<point_feature_on_canvas> g;

public:
  LabelGroup(point_feature_on_canvas pfoc): g({pfoc}) {}

  LabelGroup& push_back(point_feature_on_canvas pfoc){g.push_back(pfoc); return *this;}
  size_t size(){return g.size();}
  

  point_feature_on_canvas& operator[](unsigned int i){ return g[i]; }
  point_feature_on_canvas  operator[](unsigned int i) const { return g[i]; }

  friend ostream& operator<<(ostream& S, const LabelGroup& lg) {
    S << lg.g;
    return S;
  }

};


class LabelGroups{
  vector<LabelGroup> gg;

public:
  LabelGroups(const vector<point_feature_on_canvas> &vpfoc){
    // build
    for(size_t i=0; i<vpfoc.size(); i++){
      append_as_new_group(vpfoc[i]);
    }

    // sort by x from left to right
    sort(gg.begin(), gg.end(),
         [](const LabelGroup& lg1, const LabelGroup& lg2) {return lg1[0].x < lg2[0].x;});

    // assign x_offset relative to peak as max(x,x_prev+bb),  which is not symmetric
    for (size_t i=1; i<gg.size(); i++){ // don't shift the first one
      gg[i][0].xshift = max(0, gg[i-1][0].x + gg[i-1][0].xshift - gg[i][0].x + 22); // 15 being the space required for one label
    }

    // initial groups
    for (auto it=gg.begin(); it!=gg.end();){
      if((*it)[0].xshift != 0){
        it=merge(it-1, it);
      }else{ it++; }
    }

    // shift groups left, such that for each group the sum of all offsets is zero
    for (size_t i=0; i<gg.size(); i++){
      int group_sum_shift=0;
      for (size_t j=0; j<gg[i].size(); j++){
        group_sum_shift += gg[i][j].xshift;
      }
      for (size_t j=0; j<gg[i].size(); j++){
        gg[i][j].xshift -= group_sum_shift/gg[i].size();
      }
    }

  }


  LabelGroup& operator[](unsigned int i){ return gg[i]; }
  LabelGroup  operator[](unsigned int i) const { return gg[i]; }


  LabelGroups& append_as_new_group(point_feature_on_canvas pfoc){gg.push_back(LabelGroup(pfoc)); return *this;}
  size_t size(){return gg.size();}

  vector<LabelGroup>::iterator merge(vector<LabelGroup>::iterator it1, vector<LabelGroup>::iterator it2){
    for(size_t i=0; i<it2->size(); i++){
      it1->push_back((*it2)[i]);
    }
    return gg.erase(it2);
  }


  friend ostream& operator<<(ostream& S, const LabelGroups& lgs) {
    S << lgs.gg;
    return S;
  }
};

#endif

