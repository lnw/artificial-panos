#pragma once

#include <iostream>
#include <vector>

#include "mapitems.hh"


struct group {
  int first_index, last_index;
  int centre, width;

  friend std::ostream& operator<<(std::ostream& S, const group& g) {
    S << "{" << g.first_index << "--" << g.last_index << ", " << g.centre << ", " << g.width << "}";
    return S;
  }
};

// all labels are organised in groups, where all labels in one group touch each other
template <typename dist_t>
class LabelGroups {
  // a vector of all point features, ordered from left to right
  std::vector<point_feature_on_canvas<dist_t>> pfocs;
  // one entry per group, also ordered from left to right, so g[n].last_index + 1 = g[n+1].first_index
  std::vector<group> g;
  int canvas_width;

public:
  LabelGroups(const std::vector<point_feature_on_canvas<dist_t>>& _pfocs, int cw);

  // gather groups from indices 'first' through 'last', in a selfconsistent way
  // first and last are indices of groups (not pfocs)
  // return number of resulting groups
  int gather_groups(int first, int last);

  // assign xshifts to labels in one or more groups
  // first and last are indices of groups
  void assign_xshifts(int first, int last);

  // completely dissociate the group with index 'ind' into one-label groups
  void dissociate_group(const int64_t ind);

  // removes labels and returns them such that we have a list of which are omitted
  std::vector<point_feature_on_canvas<dist_t>> prune();

  // remove label 'index' which is in labelgroup 'lg' from LabelGroups
  void remove_label(int index, int lg);

  // subscript for the point feature, ignores groups etc
  auto& operator[](int64_t i) { return pfocs[i]; }
  auto operator[](int64_t i) const { return pfocs[i]; }
  int64_t size() { return std::ssize(pfocs); } // number of point features only

  friend std::ostream& operator<<(std::ostream& S, const LabelGroups& lg) {
    S << "[" << lg.pfocs << ", " << lg.g << "]";
    return S;
  }
};
