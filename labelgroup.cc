
#include <algorithm> // sort
#include <cassert>
#include <iostream>
#include <utility>
#include <vector>

#include "geometry.hh"
#include "labelgroup.hh"
#include "mapitems.hh"

using namespace std;

const int label_width = 18;

LabelGroups::LabelGroups(const vector<point_feature_on_canvas>& _pfocs, int cw): pfocs(_pfocs), canvas_width(cw) {
  // sort by x from left to right
  sort(pfocs.begin(), pfocs.end(),
       [](const point_feature_on_canvas& pfoc1, const point_feature_on_canvas& pfoc2) { return pfoc1.x < pfoc2.x; });
  // one new group for each label
  g.resize(pfocs.size());
  for (size_t i = 0; i < g.size(); i++) {
    g[i].first_index = i;
    g[i].last_index = i;
    g[i].centre = pfocs[i].x;
    if (g[i].centre - label_width / 2 < 0)
      g[i].centre = label_width / 2;
    if (g[i].centre + label_width / 2 > canvas_width)
      g[i].centre = canvas_width - label_width / 2;
    g[i].width = label_width; // that's one label
  }

  if (!g.empty()) {
    gather_groups(0, g.size() - 1);
    assign_xshifts(0, g.size() - 1);
  }
  // cout << "after init: " << pfocs << endl;
  // cout << "after init: " << g << endl;
}


// gather groups from indices 'first' through 'last', in a selfconsistent way
// first and last are indices of groups (not pfocs)
// return number of resulting groups
int LabelGroups::gather_groups(int first, int last) {
  assert(last < static_cast<int>(g.size()));
  assert(first <= static_cast<int>(last));
  if (last == first)
    return 1; // one element
  //cout << "gather " << first << " to " << last << " (from " << g.size()<< ")" << endl;
  bool converged = false;
  while (!converged) {
    converged = true;
    for (int ind = first; ind < last;) {
      if (g[ind].centre + g[ind].width / 2 > g[ind + 1].centre - g[ind + 1].width / 2) { // do this one and the next group overlap?
        // merge i with i+1: update the first, erase the second
        g[ind].last_index = g[ind + 1].last_index;
        g[ind].width += g[ind + 1].width;
        // find new centre
        // it->centre = (pfocs[it->first_index].x + pfocs[(it+1)->last_index].x) / 2; // works, but looks silly
        int av = 0;
        for (int i = g[ind].first_index; i <= g[ind].last_index; i++)
          av += pfocs[i].x;
        av /= (g[ind].last_index - g[ind].first_index + 1);
        g[ind].centre = av;

        // labels should not be off canvas
        if (g[ind].centre - g[ind].width / 2 < 0)
          g[ind].centre = g[ind].width / 2;
        if (g[ind].centre + g[ind].width / 2 > canvas_width)
          g[ind].centre = canvas_width - g[ind].width / 2;
        // delete i+1, erase returns an it that points to the element after the removed one
        // cout << "going to erase element " << ind+1 << " (there are " << g.size() << ")" << endl;
        g.erase(g.begin() + ind + 1);
        // do at least one more cycle
        converged = false;
        last--;
      }
      else {
        ind++;
      }
    }
  }
  // cout << "end of gather: " << first << ", " << last << endl;
  return last - first + 1;
}


// assign xshifts to labels in one or more groups
// first and last are indices of groups
void LabelGroups::assign_xshifts(int first, int last) {
  assert(last < static_cast<int>(g.size()));
  assert(first <= static_cast<int>(last));
  // cout << "assign XS " << first << " to " << last << " (from " << g.size()<< ")" << endl;
  for (int i = first; i <= last; i++) { // groups
    // cout << "pfocs from " << g[i].first_index << " to " << g[i].last_index << endl;
    for (int j = g[i].first_index; j <= g[i].last_index; j++) { // indices of pfocs
      pfocs[j].xshift = g[i].centre - pfocs[j].x + (j - g[i].first_index) * label_width - (g[i].width - label_width) / 2;
      // cout << i << "," << j << " setting xshift to " << pfocs[j].xshift << endl;
    }
  }
}


// completely dissociate the group with index 'ind' into one-label groups
void LabelGroups::dissociate_group(const size_t ind) {
  assert(ind < g.size());
  const int group_size(g[ind].last_index - g[ind].first_index + 1);
  vector<group> to_be_inserted;
  to_be_inserted.resize(group_size);
  for (int i = 0; i < group_size; i++) {
    to_be_inserted[i].first_index = g[ind].first_index + i;
    to_be_inserted[i].last_index = g[ind].first_index + i;
    to_be_inserted[i].centre = pfocs[g[ind].first_index + i].x;
    to_be_inserted[i].width = label_width;
  }
  vector<group>::iterator it = g.erase(g.begin() + ind);
  g.insert(it, to_be_inserted.begin(), to_be_inserted.end());
}


// removes labels and returns them such that we have a list of which are omitted
vector<point_feature_on_canvas> LabelGroups::prune() {
  // cout << "starting prune" << endl;
  vector<point_feature_on_canvas> removed_labels;

  // iterate over labelgroups by index (because iterators will be invalidated)
  for (size_t lg = 0; lg < g.size(); lg++) {

    while (true) {
      bool delete_something = false;
      for (int j = g[lg].first_index; j <= g[lg].last_index; j++) {
        if (abs(pfocs[j].xshift) / label_width > 4) { // one label is dispaced by more than 4 labelwidths
          delete_something = true;
        }
      }
      if (!delete_something) {
        // cout << "group " << lg << " is fine" << endl;
        break;
      }
      else {
        //cout << "one or more labels should be removed from group " << lg << endl;
      }

      if (delete_something) {
        double slope = 0;
        int ind_lower;
        for (int m = g[lg].first_index; m < g[lg].last_index; m++) { // triangular w/o diagonal
          for (int n = m + 1; n <= g[lg].last_index; n++) {
            const double e1 = pfocs[m].pf.elev,
                         e2 = pfocs[n].pf.elev;
            const double d_ele = e1 - e2;
            const double lat1 = pfocs[m].pf.lat,
                         lon1 = pfocs[m].pf.lon,
                         lat2 = pfocs[n].pf.lat,
                         lon2 = pfocs[n].pf.lon;
            const double d_dist = distance_atan(lat1, lon1, lat2, lon2);
            if (d_ele / d_dist > slope) {
              slope = d_ele / d_dist;
              ind_lower = n;
            }
            if (-d_ele / d_dist > slope) {
              slope = -d_ele / d_dist;
              ind_lower = m;
            }
          }
        }
        // cout << "deleting " << pfocs[ind_lower] << endl;
        removed_labels.push_back(pfocs[ind_lower]);
        remove_label(ind_lower, lg);
      }

      const int first_g = lg, last_g = lg + g[lg].last_index - g[lg].first_index; // because after dissociate, every pfoc has its own group
      dissociate_group(lg);
      const int n_new_groups = gather_groups(first_g, last_g);
      assign_xshifts(first_g, first_g + n_new_groups - 1);
    }
  }

  // cout << "end of prune " << pfocs << endl;
  // cout << "end of prune " << g << endl;
  return removed_labels;
}

// remove label 'index' which is in labelgroup 'lg' from LabelGroups
void LabelGroups::remove_label(int index, int lg) {
  // cout << "remove " << index << " from " << lg << " ..." << flush;
  pfocs.erase(pfocs.begin() + index);
  g[lg].last_index--;
  if (lg == static_cast<int>(g.size() - 1))
    return;
  for (int i = lg + 1; i <= static_cast<int>(g.size() - 1); i++) {
    g[i].first_index--;
    g[i].last_index--;
  }
  // cout << " done" << endl;
}
