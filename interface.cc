#include <string>
#include <vector>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "canvas.hh"
#include "scene.hh"

namespace py = pybind11;

PYBIND11_MODULE(libartpano, m) {
  m.doc() = "documentation string"; // optional

  // class scene
  py::class_<scene>(m, "scene")
      .def(py::init<double, double, double, double, double, double, double, double, std::vector<std::string>>())
      .def_static("determine_required_tiles", &scene::determine_required_tiles); // double, double, double, double, double

  // class canvas_t
  py::class_<canvas_t>(m, "canvas_t")
      .def(py::init<int, int>())
      .def("bucket_fill", &canvas_t::bucket_fill)   // int8, int8, int8
      .def("render_scene", &canvas_t::render_scene) // scene
      .def("highlight_edges", &canvas_t::highlight_edges);

  // class canvas
  py::class_<canvas>(m, "canvas")
      .def(py::init<std::string, canvas_t>())
      .def("draw_coast", &canvas::draw_coast)             // scene
      .def("annotate_peaks", &canvas::annotate_peaks)     // scene
      .def("annotate_islands", &canvas::annotate_islands) // scene
      .def("label_axis", &canvas::label_axis);            // scene
}
