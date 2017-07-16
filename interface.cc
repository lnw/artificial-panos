
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "scene.hh"
#include "canvas.hh"

namespace py = pybind11;

PYBIND11_PLUGIN(libartpano) {
  py::module m("libartpano", "pybind11 example plugin");

// class scene
  py::class_<scene>(m, "scene")
    .def(py::init<double , double , double , double , double , double , double , double >())
    .def_static("determine_required_tiles", &scene::determine_required_tiles); // double, double, double, double, double

// class canvas
  py::class_<canvas>(m, "canvas")
    .def(py::init<char const * , int , int>())
    .def("bucket_fill", &canvas::bucket_fill) // int, int, int
    .def("render_scene", &canvas::render_scene) // scene
    .def("highlight_edges", &canvas::highlight_edges)
    .def("annotate_peaks", &canvas::annotate_peaks) // scene
    .def("label_axis", &canvas::label_axis); // scene

    return m.ptr();
}

