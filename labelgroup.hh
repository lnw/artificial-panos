#ifndef LABELGROUP_HH
#define LABELGROUP_HH

#include <iostream>
#include <cmath>
#include <vector>

#include "mapitems.hh"

using namespace std;

struct group{
  size_t first_index, last_index;
  int centre, width;

  friend ostream& operator<<(ostream& S, const group& g) {
    S << "{" << g.first_index << "--" << g.last_index << ", " << g.centre << ", " << g.width << "}";
    return S;
  }
};

// all labels are organised in groups, where all labels in one group touch each other
class LabelGroups{
  // a vector of all point features, ordered from left to right
  vector<point_feature_on_canvas> pfocs;
  // one entry per group, also ordered from left to right, so g[n].last_index + 1 = g[n+1].first_index
  vector<group> g;
  int canvas_width;

public:
  LabelGroups(vector<point_feature_on_canvas> _pfocs, int cw);

  // gather groups from indices 'first' through 'last', in a selfconsistent way
  // first and last are indices of groups (not pfocs)
  // return number of resulting groups
  int gather_groups(int first, int last);

  // assign xshifts to labels in one or more groups
  // first and last are indices of groups
  void assign_xshifts(int first, int last);

  // completely dissociate the group with index 'ind' into one-label groups
  void dissociate_group(const size_t ind);

  // removes labels and returns them such that we have a list of which are omitted
  vector<point_feature_on_canvas> prune();

  // remove label 'index' which is in labelgroup 'lg' from LabelGroups
  void remove_label(int index, int lg);

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

