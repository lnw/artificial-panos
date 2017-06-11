#ifndef LABELGROUP_HH
#define LABELGROUP_HH

// #include <algorithm> // min, max
#include <iostream>
// #include <math.h>
#include <vector>

//#include "geometry.hh"
//#include "array2D.hh"
//#include "tile.hh"
#include "mapitems.hh"

using namespace std;

const int label_width=18;

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
  int canvas_width;

public:
  LabelGroups(vector<point_feature_on_canvas> _pfocs, int cw): pfocs(_pfocs), canvas_width(cw) {
    // sort by x from left to right
    sort(pfocs.begin(), pfocs.end(),
         [](const point_feature_on_canvas& pfoc1, const point_feature_on_canvas& pfoc2) {return pfoc1.x < pfoc2.x;});
    // stupid groups
    g.resize(pfocs.size());
    for(size_t i=0; i< g.size(); i++){
      g[i].start_index=i;
      g[i].end_index=i;
      g[i].centre=pfocs[i].x;
      if(g[i].centre-label_width/2 < 0) g[i].centre = label_width/2;
      if(g[i].centre+label_width/2 > canvas_width) g[i].centre = canvas_width-label_width/2;
      g[i].width=label_width; // that's one label
    }
    // assign groups
    bool converged=false;
    while(!converged){
      converged=true;
      for(vector<group>::iterator it=g.begin(); ;){ // we leave the loop manually, because of invalid pointers
        if(it->centre + it->width/2 > (it+1)->centre - (it+1)->width/2){ // do this and the next group intersect?
          // merge i with i+1
          it->end_index = (it+1)->end_index;
          it->width += (it+1)->width;
          // it->centre = (pfocs[it->start_index].x + pfocs[(it+1)->end_index].x) / 2; // works, but looks silly
          int av=0;
          for(int i=it->start_index; i<=it->end_index; i++) av += pfocs[i].x;
          av /= (it->end_index - it->start_index +1);
          it->centre = av;
          if(it->centre - it->width/2 < 0) it->centre = it->width/2;
          if(it->centre + it->width/2 > canvas_width) it->centre = canvas_width - it->width/2;
          // delete i+1
          it = g.erase(it+1) - 1;
          // do at least one more cycle
          converged=false;
          if(it+1==g.end()) break;
        }else{
          it++;
          if(it+1==g.end()) break;
        }
      }
    }
    // assign xshifts
    for(size_t i=0; i< g.size(); i++){ // groups
      for(size_t j=g[i].start_index; j<=g[i].end_index; j++){ // indices of pfocs
        pfocs[j].xshift = g[i].centre - pfocs[j].x + (j-g[i].start_index)*label_width - (g[i].width-label_width)/2;
        // cout << "setting xshift to " << pfocs[j].xshift << endl;
      }
    }
  }

  // removes labels and returns them such that we have a list of which are omitted
  vector<point_feature_on_canvas> prune(){

    for(size_t i=0; i<g.size(); i++){

      bool delete_something=false;
      for(int j=g[i].start_index; j<=g[i].end_index; j++){
        if(abs(pfocs[j].xshift)/label_width > 4) delete_something=true;
      }

    // if there are large xshifts, find large real world slopes, delete lower end
    // depending on the first and last xshift, several peaks can be deteted at the same time
    // dissociate group, rebuild

    }

    return vector<point_feature_on_canvas>();
  }

  // subscript for the point feature, ignores groups etc
  point_feature_on_canvas& operator[](unsigned int i)       { return pfocs[i]; }
  point_feature_on_canvas  operator[](unsigned int i) const { return pfocs[i]; }
  size_t size(){return pfocs.size();} // number of point features only

  friend ostream& operator<<(ostream& S, const LabelGroups& lg) {
    S << "[" << lg.pfocs << ", " << lg.g << "]";
    return S;
  }

};

#endif

