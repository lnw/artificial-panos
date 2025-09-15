#include "canvas.hh"
#include "geometry.hh"
#include "scene.hh"
#include <string>
#include <vector>

#include <boost/program_options.hpp>

namespace po = boost::program_options;

int main(int ac, char** av) {
  const float lat(47.64829), lon(10.57081), elevation(-1), view_direction_h(270), view_width(120), view_height(40), view_direction_v(0), range_km(20);
  const int canvas_width(10000), canvas_height(1500);

  po::options_description desc("options");
  // clang-format off
  desc.add_options()("help,h", "produce help message")
                    ("lat", po::value<float>(), "latitude [deg]")
                    ("lon", po::value<float>(), "longitude [deg]")
                    ("elevation", po::value<float>()->default_value(elevation), "elevation [m]")
                    ("view-dir-h", po::value<float>()->default_value(view_direction_h), "horizontal view direction [deg]")
                    ("view-dir-v", po::value<float>()->default_value(view_direction_v), "vertical view direction [deg]")
                    ("view-width", po::value<float>()->default_value(view_width), "horizontal view extent [deg]")
                    ("view-height", po::value<float>()->default_value(view_height), "vertical view extent [deg]")
                    ("canvas-width", po::value<int>()->default_value(canvas_width), "horizontal canvas size [pixels]")
                    ("canvas-height", po::value<int>()->default_value(canvas_height), "vertical canvas size [pixels]")
                    ("range", po::value<float>()->default_value(range_km), "range [km]");
  // clang-format on

  po::variables_map vm;
  po::store(po::parse_command_line(ac, av, desc), vm);
  po::notify(vm);

  if (vm.count("help")) {
    std::cout << desc << std::endl;
    return 1;
  }

  const std::vector<elevation_source> sources_to_consider({elevation_source::view1, elevation_source::srtm1, elevation_source::view3, elevation_source::srtm3});

  scene<float> S({deg2rad_v<float> * vm["lat"].as<float>(),
                  deg2rad_v<float> * vm["lon"].as<float>()},
                 vm["elevation"].as<float>(),
                 deg2rad_v<float> * vm["view-dir-h"].as<float>(),
                 deg2rad_v<float> * vm["view-width"].as<float>(),
                 deg2rad_v<float> * vm["view-dir-v"].as<float>(),
                 deg2rad_v<float> * vm["view-height"].as<float>(),
                 1000 * vm["range"].as<float>(),
                 sources_to_consider);

  const std::string filename = "out.png";

  canvas_t<float> V(vm["canvas-width"].as<int>(), vm["canvas-height"].as<int>());
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
