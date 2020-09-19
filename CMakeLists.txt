cmake_minimum_required(VERSION 3.2.0)
project(artpano)


set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PROJECT_SOURCE_DIR}/cmake)
message(STATUS "cmake mod path ${CMAKE_MODULE_PATH}")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED YES)

set(CMAKE_VERBOSE_MAKEFILE FALSE)

# set(CMAKE_CXX_CLANG_TIDY "clang-tidy;-fix")

set(CMAKE_CXX_FLAGS "-fopenmp")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -Wall -Wshadow -Wpedantic -fsanitize=undefined")

find_package(libxml++ REQUIRED)
# message(STATUS "libxml++ incl dir ${libxml++_INCLUDE_DIR}")
# message(STATUS "glibmm incl dir ${glibmm_INCLUDE_DIR}")

find_package(GD REQUIRED)
# message(STATUS "libGD libs ${GD_LIBRARY}")

if(CMAKE_VERSION VERSION_LESS "3.12.0")
  # find_package(PythonInterp 3.0 REQUIRED)
  find_package(PythonLibs 3.0 REQUIRED)
  INCLUDE_DIRECTORIES(${PYTHON_INCLUDE_DIRS})
else()
  find_package(Python3 COMPONENTS Development REQUIRED)
  INCLUDE_DIRECTORIES(${Python3_INCLUDE_DIRS})
endif()

include_directories(${glib_INCLUDE_DIR} ${glib_config_INCLUDE_DIR} ${glibmm_INCLUDE_DIR} ${glibmm_config_INCLUDE_DIR} ${libxml++_INCLUDE_DIR} ${libxml++_config_INCLUDE_DIR})
add_library(ap SHARED mapitems.cc geometry.cc canvas.cc labelgroup.cc scene.cc)
target_link_libraries(ap ${libxml++_LIBRARIES})

add_executable(pano pano.cc)
target_link_libraries(pano ap ${GD_LIBRARY})

add_library(artpano SHARED interface.cc)
target_link_libraries(artpano ap ${GD_LIBRARY})

