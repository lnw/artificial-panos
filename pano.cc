#include <string>
#include <vector>

#include "canvas.hh"
#include "geometry.hh"
#include "scene.hh"


using namespace std;

int main(int ac, char** av) {

  // const double pos_lat(59.9374*deg2rad), pos_lon(10.7168*deg2rad), pos_z(100), view_direction_h(270*deg2rad), view_width(355*deg2rad), view_height(20*deg2rad), view_direction_v(0*deg2rad), range(80000); // oslo, uni
  // const double pos_lat(49.4*deg2rad), pos_lon(8.6*deg2rad), pos_z(200), view_direction_h(50*deg2rad), view_width(355*deg2rad), view_height(15*deg2rad), view_direction_v(3*deg2rad), range(100000); // hd
  // const double pos_lat(47.3664*deg2rad), pos_lon(8.5413*deg2rad), pos_z(408), view_direction_h(290*deg2rad), view_width(60*deg2rad), view_height(12*deg2rad), view_direction_v(0*deg2rad), range(100000); // zurich
  // const double pos_lat(49.08123*deg2rad), pos_lon(19.62642*deg2rad), pos_z(585), view_direction_h(230*deg2rad), view_width(290*deg2rad), view_direction_v(7*deg2rad), view_height(20*deg2rad), range(40000); // some place in slovakia
  // const double pos_lat(59.9180*deg2rad), pos_lon(10.5154*deg2rad), pos_z(400), view_direction_h(180*deg2rad), view_width(355*deg2rad), view_direction_v(0*deg2rad), view_height(12*deg2rad), range(10000); // rad, rad, m
  // const double pos_lat(59.9851*deg2rad), pos_lon(6.0173*deg2rad), pos_z(-1), view_direction_h(0*deg2rad), view_width(355*deg2rad), view_height(50*deg2rad), view_direction_v(10*deg2rad), range(50000); // rosendal
  // const double pos_lat(48.93885*deg2rad), pos_lon(8.61177*deg2rad), pos_z(300), view_direction_h(220*deg2rad), view_width(355*deg2rad), view_height(30*deg2rad), view_direction_v(0*deg2rad), range(70000); // a garden
  // const double pos_lat(49.38002*deg2rad), pos_lon(8.66683*deg2rad), pos_z(130), view_direction_h(0*deg2rad), view_width(355*deg2rad), view_height(30*deg2rad), view_direction_v(5*deg2rad), range(100000); // kirchheim
  // const double pos_lat(49.4105*deg2rad), pos_lon(8.6766*deg2rad), pos_z(130), view_direction_h(0*deg2rad), view_width(355*deg2rad), view_height(30*deg2rad    ), view_direction_v(5*deg2rad), range(100000); // bruecke
  // const double pos_lat(58.2477 * deg2rad), pos_lon(6.5597 * deg2rad), pos_z(-1), view_direction_h(280 * deg2rad), view_width(280 * deg2rad), view_height(40 * deg2rad), view_direction_v(0), range(10000); // south norway
  const double pos_lat(44.85029 * deg2rad), pos_lon(7.19331 * deg2rad), pos_z(2000), view_direction_h(120 * deg2rad), view_width(210 * deg2rad), view_height(60 * deg2rad), view_direction_v(0), range(50000); // near turin

  const std::vector<elevation_source> sources_to_consider({elevation_source::view1, elevation_source::srtm1, elevation_source::view3, elevation_source::srtm3});

  scene S({pos_lat, pos_lon}, pos_z, view_direction_h, view_width, view_direction_v, view_height, range, sources_to_consider);

  const std::string filename = "out.png";
  const int view_x(10000), view_y(1500); // pixels

  canvas_t<double> V(view_x, view_y);
  V.bucket_fill(100, 100, 100);
  V.render_scene(S);
  V.highlight_edges();

  canvas<double> VV(filename, std::move(V));
  // VV.draw_coast(S);
  VV.annotate_peaks(S);
  VV.label_axis(S);
  VV.write_png();

  return 0;
}
