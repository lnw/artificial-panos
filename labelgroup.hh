#ifndef LABELGROUP_HH
#define LABELGROUP_HH

// #include <algorithm> // min, max
#include <iostream>
// #include <math.h>
#include <vector>

#include "mapitems.hh"

using namespace std;

const int label_width=18;

struct group{
  int start_index, end_index;
  int centre, width;

  friend ostream& operator<<(ostream& S, const group& g) {
    S << "{" << g.start_index << "--" << g.end_index << ", " << g.centre << ", " << g.width << "}";
    return S;
  }
};

// all labels are organised in groups, where are labels in one group touch each other
class LabelGroups{
  // a vector of all point features, ordered from left to right
  vector<point_feature_on_canvas> pfocs;
  // one entry per group, also ordered from left to right, so g[n].end_index + 1 = g[n+1].start_index
  vector<group> g;
  int canvas_width;

public:
  LabelGroups(vector<point_feature_on_canvas> _pfocs, int cw): pfocs(_pfocs), canvas_width(cw) {
    // sort by x from left to right
    sort(pfocs.begin(), pfocs.end(),
         [](const point_feature_on_canvas& pfoc1, const point_feature_on_canvas& pfoc2) {return pfoc1.x < pfoc2.x;});
    // one new group for each label
    g.resize(pfocs.size());
    for(size_t i=0; i< g.size(); i++){
      g[i].start_index=i;
      g[i].end_index=i;
      g[i].centre=pfocs[i].x;
      if(g[i].centre-label_width/2 < 0) g[i].centre = label_width/2;
      if(g[i].centre+label_width/2 > canvas_width) g[i].centre = canvas_width-label_width/2;
      g[i].width=label_width; // that's one label
    }
    
    gather_groups(0, g.size()-1);
    assign_xshifts(0, g.size()-1);
  }

  
  // gather groups from indices 'first' through 'last', in a selfconsistent way
  void gather_groups(int first, int last){
    if(last - first < 1) return;
    bool converged=false;
    while(!converged){
      converged=true;
      for(vector<group>::iterator it=g.begin()+first; ;){ // we leave the loop manually, because of invalid pointers
        if(it->centre + it->width/2 > (it+1)->centre - (it+1)->width/2){ // do this and the next group intersect?
          // merge i with i+1: update the first, erase the second
          it->end_index = (it+1)->end_index;
          it->width += (it+1)->width;
          // it->centre = (pfocs[it->start_index].x + pfocs[(it+1)->end_index].x) / 2; // works, but looks silly
          int av=0;
          for(int i=it->start_index; i<=it->end_index; i++) av += pfocs[i].x;
          av /= (it->end_index - it->start_index +1);
          it->centre = av;

          // labels should not be off canvas
          if(it->centre - it->width/2 < 0) it->centre = it->width/2;
          if(it->centre + it->width/2 > canvas_width) it->centre = canvas_width - it->width/2;
          // delete i+1
          it = g.erase(it+1) - 1;
          // do at least one more cycle
          converged=false;
          last--;
          if(it == g.begin()+last) break;
        }else{
          it++;
          if(it == g.begin()+last) break;
        }
      }
    }
  }

  // assign xshifts to labels in one or more groups
  void assign_xshifts(int first, int last){
    for(size_t i=first; i<=last; i++){ // groups
      for(size_t j=g[i].start_index; j<=g[i].end_index; j++){ // indices of pfocs
        pfocs[j].xshift = g[i].centre - pfocs[j].x + (j-g[i].start_index)*label_width - (g[i].width-label_width)/2;
        // cout << "setting xshift to " << pfocs[j].xshift << endl;
      }
    }
  }


  // completely dissociate the group with index 'ind' into one-label groups
  void dissociate_group(const size_t ind){
    assert(ind<g.size());
    const int group_size(g[ind].end_index - g[ind].start_index + 1);
    vector<group> to_be_inserted;
    to_be_inserted.resize(group_size);
    for(int i=0; i<group_size; i++){
      to_be_inserted[i].start_index = g[ind].start_index + i;
      to_be_inserted[i].end_index = g[ind].start_index + i;
      to_be_inserted[i].centre = pfocs[g[ind].start_index + i].x;
      to_be_inserted[i].width = label_width;
    }
    vector<group>::iterator it = g.erase(g.begin() + ind);
    g.insert(it, to_be_inserted.begin(), to_be_inserted.end());
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

