#include <string>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "canvas.hh"
#include "scene.hh"

namespace py = pybind11;

PYBIND11_MODULE(libartpano, m) {
  m.doc() = "documentation string"; // optional

  // class scene
  py::class_<scene>(m, "scene")
      .def(py::init<double, double, double, double, double, double, double, double, vector<string>>())
      .def_static("determine_required_tiles", &scene::determine_required_tiles); // double, double, double, double, double

  // class canvas
  py::class_<canvas>(m, "canvas")
      .def(py::init<const string, int, int>())
      .def("bucket_fill", &canvas::bucket_fill)   // int, int, int
      .def("render_scene", &canvas::render_scene) // scene
      .def("construct_image", &canvas::construct_image)
      .def("highlight_edges", &canvas::highlight_edges)
      .def("draw_coast", &canvas::draw_coast)             // scene
      .def("annotate_peaks", &canvas::annotate_peaks)     // scene
      .def("annotate_islands", &canvas::annotate_islands) // scene
      .def("label_axis", &canvas::label_axis);            // scene
}
