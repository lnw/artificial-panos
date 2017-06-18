
#include <fstream>
#include <iostream>
#include <string>

#include "auxiliary.hh"
#include "geometry.hh"
#include "array2D.hh"
#include "tile.hh"
#include "scene.hh"
#include "canvas.hh"

#include "circ_360.hh"

using namespace std;

int main(int ac, char **av) {

  // const double pos_lat(59.9374*deg2rad), pos_lon(10.7168*deg2rad), pos_z(100), view_direction_h(270*deg2rad), view_width(355*deg2rad), view_height(20*deg2rad), view_direction_v(0*deg2rad), range(80000); // oslo, uni
  const double pos_lat(49.4*deg2rad), pos_lon(8.6*deg2rad), pos_z(200), view_direction_h(50*deg2rad), view_width(355*deg2rad), view_height(15*deg2rad), view_direction_v(3*deg2rad), range(100000); // hd
  // const double pos_lat(47.3664*deg2rad), pos_lon(8.5413*deg2rad), pos_z(408), view_direction_h(290*deg2rad), view_width(60*deg2rad), view_height(12*deg2rad), view_direction_v(0*deg2rad), range(100000); // rad, rad, m // zurich
  // const double pos_lat(49.08123*deg2rad), pos_lon(19.62642*deg2rad), pos_z(585), view_direction_h(230*deg2rad), view_width(290*deg2rad), view_direction_v(7*deg2rad), view_height(20*deg2rad), range(40000); // slovakia
  // const double pos_lat(59.9180*deg2rad), pos_lon(10.5154*deg2rad), pos_z(400), view_direction_h(180*deg2rad), view_width(355*deg2rad), view_direction_v(0*deg2rad), view_height(12*deg2rad), range(10000); // rad, rad, m
  // const double pos_lat(58.0472*deg2rad), pos_lon(11.8252*deg2rad), pos_z(200), view_direction_h(180*deg2rad), view_width(355*deg2rad), view_height(15*deg2rad), view_direction_v(0*deg2rad), range(20000);
  // const double pos_lat(59.9851*deg2rad), pos_lon(6.0173*deg2rad), pos_z(30), view_direction_h(0*deg2rad), view_width(355*deg2rad), view_height(50*deg2rad), view_direction_v(10*deg2rad), range(50000); // rosendal

  scene S(pos_lat, pos_lon, pos_z, view_direction_h, view_width, view_direction_v, view_height, range);
//  cout << S.tiles[0].first << endl;
//  cout << S.tiles[0].second << endl;
//  cout << S.tiles[1].first << endl;
//  cout << S.tiles[1].second << endl;
//  cerr << "enough" << endl;


  char const * filename = "out.png";
  const int view_x(10000), view_y(1000); // pixels
  canvas V(filename, view_x, view_y);

  V.bucket_fill(100,100,100);
  V.render_scene(S);
  V.highlight_edges();
  V.annotate_peaks(S, "mapItems/peaks-bw.osm");
  // V.annotate_peaks(S, "mapItems/peaks-no_S.osm");
  // V.annotate_peaks(S, "mapItems/peaks-sw_S.osm");
  V.label_axis(S);

//  cout << V.zbuffer << endl;

  // iterate over landscape squares, maintain z-buffer
  // possibly transform z axis of picture:  viewfinder compresses forground and stretches region around horizon by 130%-200%
  // a) first paint each pixel according to the distance
  // b) hue according to distance, darker for surfaces that almost contain the location of the viewer


  return 0;
}

