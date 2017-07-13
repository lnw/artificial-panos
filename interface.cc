
#include <pybind11/pybind11.h>

namespace py = pybind11;

int add(int i, int j) {
    return i + j;
}

PYBIND11_PLUGIN(libartpano) {
    py::module m("libartpano", "pybind11 example plugin");

    m.def("add", &add, "A function which adds two numbers");

    return m.ptr();
}

