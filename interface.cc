#include <string>
#include <vector>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "canvas.hh"
#include "latlon.hh"
#include "scene.hh"

namespace py = pybind11;

PYBIND11_MODULE(libartpano, m) {
  m.doc() = "documentation string"; // optional

  py::class_<LatLon<double, Unit::rad>>(m, "latlondouble")
      .def(py::init<double, double>());
  py::class_<LatLon<int64_t, Unit::deg>>(m, "latlonint")
      .def(py::init<int64_t, int64_t>());

  m.def("vll2vp_int64", &vll2vp);

  py::enum_<elevation_source>(m, "elevation_source")
      .value("srtm1", elevation_source::srtm1)
      .value("srtm3", elevation_source::srtm3)
      .value("view1", elevation_source::view1)
      .value("view3", elevation_source::view3);

  // class scene
  py::class_<scene>(m, "scene")
      .def(py::init<LatLon<double, Unit::rad>, double, double, double, double, double, double, std::vector<elevation_source>>())
      .def_static("determine_required_tiles", &scene::determine_required_tiles_v); // double, double, double, latlon

  // class canvas_t
  using canvas_t_type = canvas_t<double>;
  py::class_<canvas_t_type>(m, "canvas_t")
      .def(py::init<int, int>())
      .def("bucket_fill", &canvas_t_type::bucket_fill)   // int8, int8, int8
      .def("render_scene", &canvas_t_type::render_scene) // scene
      .def("highlight_edges", &canvas_t_type::highlight_edges);

  // class canvas
  using canvas_type = canvas<double>;
  py::class_<canvas_type>(m, "canvas")
      .def(py::init<std::string, canvas_t_type>())
      .def("draw_coast", &canvas_type::draw_coast)             // scene
      .def("annotate_peaks", &canvas_type::annotate_peaks)     // scene
      .def("annotate_islands", &canvas_type::annotate_islands) // scene
      .def("label_axis", &canvas_type::label_axis)             // scene
      .def("write_png", &canvas_type::write_png);
}
