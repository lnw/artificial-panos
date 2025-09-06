#include "canvas.hh"
#include "geometry.hh"
#include "scene.hh"
#include <string>
#include <vector>

#include <boost/program_options.hpp>

namespace po = boost::program_options;

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
  // const float pos_lat(44.85029 * deg2rad), pos_lon(7.19331 * deg2rad), pos_z(2000), view_direction_h(120 * deg2rad), view_width(210 * deg2rad), view_height(60 * deg2rad), view_direction_v(0), range(50000); // near turin
  // const float pos_lat(61.50184 * deg2rad), pos_lon(8.71880 * deg2rad), pos_z(-1), view_direction_h(190 * deg2rad), view_width(120 * deg2rad), view_height(40 * deg2rad), view_direction_v(0), range(50000); // near turin
  const float pos_lat(47.64829), pos_lon(10.57081), pos_z(-1), view_direction_h(270), view_width(120), view_height(40), view_direction_v(0), range(100); // bayern

  po::options_description desc("options");
  // clang-format off
  desc.add_options()("help", "produce help message")
                    ("pos_lat", po::value<float>(), "latitude [deg]")
                    ("pos_lon", po::value<float>(), "longitude [deg]")
                    ("pos_z", po::value<float>()->default_value(pos_z), "elevation [m]")
                    ("view_direction_h", po::value<float>()->default_value(view_direction_h), "horizontal view direction [deg]")
                    ("view_direction_v", po::value<float>()->default_value(view_direction_v), "vertical view direction [deg]")
                    ("view_width", po::value<float>()->default_value(view_width), "horizontal view extent [deg]")
                    ("view_height", po::value<float>()->default_value(view_height), "vertical view extent [deg]")
                    ("range", po::value<float>()->default_value(range), "range [km]");
  // clang-format on

  po::variables_map vm;
  po::store(po::parse_command_line(ac, av, desc), vm);
  po::notify(vm);

  if (vm.count("help")) {
    std::cout << desc << std::endl;
    return 1;
  }

  const std::vector<elevation_source> sources_to_consider({elevation_source::view1, elevation_source::srtm1, elevation_source::view3, elevation_source::srtm3});

  scene<float> S({deg2rad_v<float> * vm["pos_lat"].as<float>(),
                  deg2rad_v<float> * vm["pos_lon"].as<float>()},
                 vm["pos_z"].as<float>(),
                 deg2rad_v<float> * vm["view_direction_h"].as<float>(),
                 deg2rad_v<float> * vm["view_width"].as<float>(),
                 deg2rad_v<float> * vm["view_direction_v"].as<float>(),
                 deg2rad_v<float> * vm["view_height"].as<float>(),
                 1000 * vm["range"].as<float>(),
                 sources_to_consider);

  const std::string filename = "out.png";
  const int view_x(10000), view_y(1500); // pixels

  canvas_t<float> V(view_x, view_y);
  V.bucket_fill(100, 100, 100);
  V.render_scene(S);
  V.highlight_edges();

  canvas<float> VV(filename, std::move(V));
  // VV.draw_coast(S);
  VV.annotate_peaks(S);
  VV.label_axis(S);
  VV.write_png();

  return 0;
}
