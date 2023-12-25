cmake_minimum_required(VERSION 3.20.0)
project(artpano CXX)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PROJECT_SOURCE_DIR}/cmake)
message(STATUS "cmake mod path ${CMAKE_MODULE_PATH}")

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED YES)
set(CMAKE_VERBOSE_MAKEFILE false)

# set(CMAKE_CXX_CLANG_TIDY "clang-tidy;-fix")

find_package(OpenMP REQUIRED)

set(CMAKE_CXX_FLAGS "-march=native")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -Wall -Wshadow -Wpedantic -fsanitize=undefined")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}")

find_package(tinyxml2 REQUIRED)

find_package(GD REQUIRED)
# message(STATUS "libGD libs ${GD_LIBRARY}")

find_package(Python3 COMPONENTS Development REQUIRED)

add_library(ap SHARED
            array2d.hh
            auxiliary.hh
            canvas.cc
            canvas.hh
            colour.hh
            degrad.hh
            geometry.hh
            labelgroup.cc
            labelgroup.hh
            latlon.hh
            mapitems.cc
            mapitems.hh
            scene.cc
            scene.hh
            tile.hh
)
# target_compile_definitions(ap PRIVATE GRAPHICS_DEBUG)
target_link_libraries(ap tinyxml2)
target_link_libraries(ap ${GD_LIBRARY})
target_link_libraries(ap OpenMP::OpenMP_CXX)

include(CheckIPOSupported)
check_ipo_supported(RESULT DO_LTO OUTPUT LTO_MESSAGE)
if(DO_LTO)
  set_property(TARGET ap PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
else()
  message(WARNING "IPO is not supported: ${LTO_MESSAGE}")
endif()

add_executable(pano pano.cc)
target_link_libraries(pano PRIVATE ap)

add_library(artpano SHARED
            interface.cc
)
target_link_libraries(artpano PRIVATE ap)
target_link_libraries(artpano PUBLIC Python3::Module)

