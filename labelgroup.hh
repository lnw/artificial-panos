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

const int label_width=20;

struct group {
  int start_index, end_index;
  int centre, width;

  friend ostream& operator<<(ostream& S, const group& g) {
    S << "{" << g.start_index << "--" << g.end_index << ", " << g.centre << ", " << g.width << "}";
    return S;
  }
};

class LabelGroups{
  vector<point_feature_on_canvas> pfocs;
  vector<group> g;

public:
  LabelGroups(vector<point_feature_on_canvas> _pfocs): pfocs(_pfocs) {
    // sort by x from left to right
    sort(pfocs.begin(), pfocs.end(),
         [](const point_feature_on_canvas& pfoc1, const point_feature_on_canvas& pfoc2) {return pfoc1.x < pfoc2.x;});
    // stupid groups
    g.resize(pfocs.size());
    for(int i=0; i< g.size(); i++){
      g[i].start_index=i;
      g[i].end_index=i;
      g[i].centre=pfocs[i].x;
      g[i].width=label_width; // that's one label
    }
cout << pfocs << endl;
    // assign groups
    bool converged=false;
    while(!converged){
      converged=true;
      for(vector<group>::iterator it=g.begin(); ;){
        if(it->centre + it->width/2 > (it+1)->centre - (it+1)->width/2){ // do this and the next group intersect?
          // merge
cout << "attempting to merge " << *it << " and " << *(it+1) << endl;
          it->end_index=(it+1)->end_index;
          it->width += (it+1)->width;
          it->centre = (pfocs[it->start_index].x + pfocs[(it+1)->end_index].x) / 2;
          //delete the next one
          it = g.erase(it+1) - 1;
cout << "next is " << *it << endl;
          // do at least one more cycle
          converged=false;
          if(it+1==g.end()) break;
        }else{
          it++;
          if(it+1==g.end()) break;
cout << "next is " << *it << endl;
        }
      }
    }
    // assign xshifts
    for(int i=0; i< g.size(); i++){ // groups
      for(int j=g[i].start_index; j<=g[i].end_index; j++){ // indices of pfocs
        pfocs[j].xshift = g[i].centre - pfocs[j].x + (j-g[i].start_index)*label_width - (g[i].width-label_width)/2;
cout << "setting xshift to " << pfocs[j].xshift << endl;
      }
    }
  }

  // subscript for the point feature, ignores groups etc
  point_feature_on_canvas& operator[](unsigned int i)       { return pfocs[i]; }
  point_feature_on_canvas  operator[](unsigned int i) const { return pfocs[i]; }
  size_t size(){return pfocs.size();}

  friend ostream& operator<<(ostream& S, const LabelGroups& lg) {
    S << "[" << lg.pfocs << ", " << lg.g << "]";
    return S;
  }

};

#endif

