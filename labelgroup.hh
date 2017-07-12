#ifndef LABELGROUP_HH
#define LABELGROUP_HH

#include <iostream>
#include <math.h>
#include <vector>

#include "mapitems.hh"

using namespace std;

const int label_width=18;

struct group{
  size_t first_index, last_index;
  int centre, width;

  friend ostream& operator<<(ostream& S, const group& g) {
    S << "{" << g.first_index << "--" << g.last_index << ", " << g.centre << ", " << g.width << "}";
    return S;
  }
};

// all labels are organised in groups, where are labels in one group touch each other
class LabelGroups{
  // a vector of all point features, ordered from left to right
  vector<point_feature_on_canvas> pfocs;
  // one entry per group, also ordered from left to right, so g[n].last_index + 1 = g[n+1].first_index
  vector<group> g;
  int canvas_width;

public:
  LabelGroups(vector<point_feature_on_canvas> _pfocs, int cw): pfocs(_pfocs), canvas_width(cw) {
    // sort by x from left to right
    sort(pfocs.begin(), pfocs.end(),
         [](const point_feature_on_canvas& pfoc1, const point_feature_on_canvas& pfoc2) {return pfoc1.x < pfoc2.x;});
    // one new group for each label
    g.resize(pfocs.size());
    for(size_t i=0; i<g.size(); i++){
      g[i].first_index=i;
      g[i].last_index=i;
      g[i].centre=pfocs[i].x;
      if(g[i].centre-label_width/2 < 0) g[i].centre = label_width/2;
      if(g[i].centre+label_width/2 > canvas_width) g[i].centre = canvas_width-label_width/2;
      g[i].width=label_width; // that's one label
    }

    gather_groups(0, g.size()-1);
    assign_xshifts(0, g.size()-1);
  }


  // gather groups from indices 'first' through 'last', in a selfconsistent way
  // first and last are indices of groups (not pfocs)
  void gather_groups(int first, int last){
    if(last - first < 1) return;
    //cout << "gather " << first << " to " << last << " (from " << g.size()<< ")" << endl;
    bool converged=false;
    while(!converged){
      converged=true;
      for(int ind=first; ind<last; ){
        if(g[ind].centre + g[ind].width/2 > g[ind+1].centre - g[ind+1].width/2){ // do this one and the next group overlap?
          // merge i with i+1: update the first, erase the second
          g[ind].last_index = g[ind+1].last_index;
          g[ind].width += g[ind+1].width;
          // find new centre
          // it->centre = (pfocs[it->first_index].x + pfocs[(it+1)->last_index].x) / 2; // works, but looks silly
          int av=0;
          for(size_t i=g[ind].first_index; i<=g[ind].last_index; i++) av += pfocs[i].x;
          av /= (g[ind].last_index - g[ind].first_index +1);
          g[ind].centre = av;

          // labels should not be off canvas
          if(g[ind].centre - g[ind].width/2 < 0) g[ind].centre = g[ind].width/2;
          if(g[ind].centre + g[ind].width/2 > canvas_width) g[ind].centre = canvas_width - g[ind].width/2;
          // delete i+1, erase returns an it that points to the element after the removed one
          g.erase(g.begin()+ind+1);
          // do at least one more cycle
          converged=false;
          last--;
        }else{
          ind++;
        }
      }
    }
  }


  // assign xshifts to labels in one or more groups
  void assign_xshifts(int first, int last){
    if(last - first < 1) return;
    // cout << "assign XS " << first << " to " << last << " (from " << g.size()<< ")" << endl;
    for(int i=first; i<=last; i++){ // groups
      for(int j=g[i].first_index; j<=g[i].last_index; j++){ // indices of pfocs
        pfocs[j].xshift = g[i].centre - pfocs[j].x + (j-g[i].first_index)*label_width - (g[i].width-label_width)/2;
        // cout << "setting xshift to " << pfocs[j].xshift << endl;
      }
    }
  }


  // completely dissociate the group with index 'ind' into one-label groups
  void dissociate_group(const size_t ind){
    assert(ind<g.size());
    const int group_size(g[ind].last_index - g[ind].first_index + 1);
    vector<group> to_be_inserted;
    to_be_inserted.resize(group_size);
    for(int i=0; i<group_size; i++){
      to_be_inserted[i].first_index = g[ind].first_index + i;
      to_be_inserted[i].last_index = g[ind].first_index + i;
      to_be_inserted[i].centre = pfocs[g[ind].first_index + i].x;
      to_be_inserted[i].width = label_width;
    }
    vector<group>::iterator it = g.erase(g.begin() + ind);
    g.insert(it, to_be_inserted.begin(), to_be_inserted.end());
  }


  // removes labels and returns them such that we have a list of which are omitted
  vector<point_feature_on_canvas> prune(){
    cout << "starting prune" << endl;
    vector<point_feature_on_canvas> removed_labels;

    // iterate over labelgroups by index (because iterators will be invalidated)
    for(size_t lg=0; lg<g.size(); lg++){

      while(true){
        bool delete_something=false;
        for(int j=g[lg].first_index; j<=g[lg].last_index; j++){
          if(abs(pfocs[j].xshift)/label_width > 4){ // one label is dispaced by more than 4 labelwidths
            delete_something=true;
          }
        }
        if(!delete_something){
          // cout << "group " << lg << " is fine" << endl;
          break;
        }else{
          cout << "one or more labels should be removed from group " << lg << endl;
        }

        if(delete_something){
          double slope = 0;
          int ind_lower;
          for(int m=g[lg].first_index; m<g[lg].last_index; m++){ // triangular w/o diagonal
            for(int n=m+1; n<=g[lg].last_index; n++){
              const double e1 = pfocs[m].pf.elev,
                           e2 = pfocs[n].pf.elev;
              const double d_ele = e1-e2;
              const double lat1 = pfocs[m].pf.lat,
                           lon1 = pfocs[m].pf.lon,
                           lat2 = pfocs[n].pf.lat,
                           lon2 = pfocs[n].pf.lon;
              const double d_dist = distance_atan(lat1, lon1, lat2, lon2);
              if(d_ele/d_dist>slope){
                slope = d_ele/d_dist;
                ind_lower = n;
              }
              if(-d_ele/d_dist>slope){
                slope = -d_ele/d_dist;
                ind_lower = m;
              }
            }
          }
          cout << "deleting " << pfocs[ind_lower] << endl;
          removed_labels.push_back(pfocs[ind_lower]);
          remove_label(ind_lower,lg);
        }

        const int first_g=lg, last_g=lg + g[lg].last_index - g[lg].first_index;
        dissociate_group(lg);
        gather_groups(first_g, last_g);
        assign_xshifts(first_g, last_g);
      }
    }

    return removed_labels;
  }

  // remove label 'index' which is in labelgroup 'lg' from LabelGroups
  void remove_label(int index, int lg){
    // cout << "remove " << index << " from " << lg << " ..." << flush;
    pfocs.erase(pfocs.begin()+index);
    g[lg].last_index--;
    if(lg==g.size()-1) return;
    for(int i=lg+1; i<=g.size()-1; i++){
      g[i].first_index--;
      g[i].last_index--;
    }
    cout << " done" << endl;
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

