cmake_minimum_required(VERSION 3.26)
project(artpano CXX)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PROJECT_SOURCE_DIR}/cmake)
message(STATUS "cmake mod path ${CMAKE_MODULE_PATH}")

set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED YES)
set(CMAKE_VERBOSE_MAKEFILE YES)

# set(CMAKE_CXX_CLANG_TIDY "clang-tidy;-fix")

find_package(OpenMP REQUIRED)
# message(STATUS "found C++ compiler with openmp version: ${OpenMP_CXX_VERSION}")

add_library(compiler_options INTERFACE)
target_compile_options(compiler_options INTERFACE
  "-march=native"
  # "-stdlib=libc++"
  "$<$<CONFIG:Debug>:-O0>"
  "$<$<CONFIG:Debug>:-Wall>"
  "$<$<CONFIG:Debug>:-Wshadow>"
  "$<$<CONFIG:Debug>:-Wpedantic>"
  "$<$<CONFIG:Debug>:-fsanitize=undefined>"
  # "$<$<CONFIG:Debug>:-fsanitize=thread>"
)
target_link_options(compiler_options INTERFACE
  "$<$<CONFIG:Debug>:-fsanitize=undefined>"
  # "$<$<CONFIG:Debug>:-fsanitize=thread>"
)

find_package(tinyxml2 REQUIRED)

find_package(GD REQUIRED)
# message(STATUS "libGD libs ${GD_LIBRARY}")

find_package(Python3 COMPONENTS Development REQUIRED)

find_package(Boost CONFIG COMPONENTS program_options)

add_library(ap SHARED)
target_sources(ap PRIVATE
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
target_link_libraries(ap
                      PUBLIC
                      ${GD_LIBRARY}
                      PRIVATE
                      tinyxml2
                      OpenMP::OpenMP_CXX
                      compiler_options
)

add_executable(gdtest gdtest.cc)
target_link_libraries(gdtest ${GD_LIBRARY})

include(CheckIPOSupported)
check_ipo_supported(RESULT DO_LTO OUTPUT LTO_MESSAGE)
if(DO_LTO)
  set_property(TARGET ap PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
else()
  message(WARNING "IPO is not supported: ${LTO_MESSAGE}")
endif()

add_executable(pano pano.cc)
target_link_libraries(pano
                      PRIVATE
                      ap
                      compiler_options
                      Boost::program_options
)
set_target_properties(pano PROPERTIES OUTPUT_NAME "artpano")

add_library(artpano SHARED)
target_sources(artpano
               PRIVATE
               interface.cc
)
target_link_libraries(artpano
                      PUBLIC
                      Python3::Module
                      PRIVATE
                      ap
                      compiler_options
)

