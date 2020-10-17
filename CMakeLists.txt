# The MIT License (MIT)
#
# Copyright (c) 2019 Train IT
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.
#

cmake_minimum_required(VERSION 3.14)
project(irrlicht-engine
  VERSION 1.0.0
  LANGUAGES CXX
)

# dependencies
find_package(irrlicht CONFIG REQUIRED)
find_package(observer-ptr-lite CONFIG REQUIRED)
find_package(gsl-lite CONFIG REQUIRED)

# build definition
add_library(irrlicht-engine STATIC
  src/engine.cpp include/irrlicht-engine/engine.h
  src/utils.cpp include/irrlicht-engine/utils.h
)
target_compile_features(irrlicht-engine PUBLIC cxx_std_20)
target_include_directories(irrlicht-engine PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>)
target_link_libraries(irrlicht-engine
  PUBLIC
  irrlicht::irrlicht
  nonstd::observer-ptr-lite
  gsl::gsl-lite)
set_target_properties(irrlicht-engine PROPERTIES EXPORT_NAME engine)
add_library(irrlicht::engine ALIAS irrlicht-engine)

# installation
include(GNUInstallDirs)

install(TARGETS irrlicht-engine EXPORT irrlicht-engine-targets
  INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)
install(DIRECTORY include/irrlicht-engine TYPE INCLUDE)

configure_file(irrlicht-engine-config.cmake.in irrlicht-engine-config.cmake @ONLY)
include(CMakePackageConfigHelpers)
write_basic_package_version_file(irrlicht-engine-config-version.cmake COMPATIBILITY SameMajorVersion)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/irrlicht-engine-config.cmake ${CMAKE_CURRENT_BINARY_DIR}/irrlicht-engine-config-version.cmake
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/irrlicht-engine
)
install(EXPORT irrlicht-engine-targets
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/irrlicht-engine
  NAMESPACE irrlicht::
)
