
# Copyright (c) 2010-2015 Stefan Eilemann <eile@eyescale.ch>
#                         Daniel Pfeifer <daniel@pfeifer-mail.de>

cmake_minimum_required(VERSION 3.1 FATAL_ERROR)
project(Equalizer VERSION 1.13.0)
set(Equalizer_VERSION_ABI 193)

list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/CMake
                              ${CMAKE_SOURCE_DIR}/CMake/common)

include(GitExternal)

set(EQUALIZER_INCLUDE_NAME eq)
set(EQUALIZER_DEB_DEPENDS bison flex libboost-program-options-dev
  libboost-filesystem-dev libboost-system-dev libboost-thread-dev
  libx11-dev libgl1-mesa-dev libglewmx1.6-dev libspnav-dev
  libopenscenegraph-dev libopencv-dev qtbase5-dev)
set(EQUALIZER_PORT_DEPENDS boost opencv)

include(Common)

option(EQUALIZER_USE_OSG "Test for OpenSceneGraph support" ON)
option(EQUALIZER_USE_HWLOC "Test for hwloc support (experimental)" OFF)
option(EQUALIZER_INSTALL_SERVER_HEADERS "Install Equalizer server headers" OFF)
option(EQUALIZER_BUILD_2_0_API
  "Enable for pure 2.0 API (breaks compatibility with 1.x API)" OFF)
option(EQUALIZER_PREFER_QT "Prefer Qt window system over X11 on OS X" OFF)

mark_as_advanced(EQUALIZER_INSTALL_SERVER_HEADERS)

set(DPUT_HOST ppa:bbp/ppa)

set(COMMON_PROJECT_DOMAIN ch.eyescale)
set(EQUALIZER_DESCRIPTION "Parallel Rendering Framework")
set(EQUALIZER_MAINTAINER "Stefan Eilemann <eile@eyescale.ch>")
set(EQUALIZER_LICENSE LGPL BSD)

if(EQUALIZER_BUILD_2_0_API)
  list(APPEND COMMON_FIND_PACKAGE_DEFINES EQ_2_0_API)
else()
  list(APPEND COMMON_FIND_PACKAGE_DEFINES EQ_1_0_API)
endif()
if(APPLE AND CMAKE_OSX_ARCHITECTURES MATCHES "i386")
  set(EQ_AGL_USED 1)
endif()

include(configure)

list(APPEND CPPCHECK_EXTRA_ARGS -DEQUALIZERFABRIC_STATIC=)

if(MSVC)
  include(UseGnuWin32)
else()
  set(PTHREAD_LIBRARIES pthread)
endif()

if(APPLE)
  add_definitions(-DDarwin)
endif(APPLE)

list(APPEND EQ_DEFINITIONS -DEQ_BUILD_DIR="${CMAKE_BINARY_DIR}/")

if(EQUALIZER_BUILD_2_0_API)
  list(APPEND EQ_DEFINITIONS -DEQ_2_0_API)
endif()
add_definitions(${EQ_DEFINITIONS})

include(GLEW/CMakeLists.txt)
add_subdirectory(eq)
add_subdirectory(seq)
add_subdirectory(tools) # before examples, they depend on it
add_subdirectory(examples)
if(EXISTS "${Equalizer_SOURCE_DIR}/tests/CMakeLists.txt" )
  add_subdirectory(tests)
endif()

if(EXISTS "${CMAKE_SOURCE_DIR}/auxiliary" AND
   IS_DIRECTORY "${CMAKE_SOURCE_DIR}/auxiliary")
  add_subdirectory(auxiliary)
endif()
add_subdirectory(doc)

set(EQUALIZER_DEPENDENT_LIBRARIES vmmlib Lunchbox Collage)
if(GLEW_MX_FOUND)
  list(APPEND EQUALIZER_DEPENDENT_LIBRARIES GLEW_MX)
endif()

common_install(RELATIVE
  FILES ${CMAKE_SOURCE_DIR}/CMake/common/FindGLEW_MX.cmake
    CMake/EqGLLibraries.cmake
  DESTINATION ${CMAKE_MODULE_INSTALL_PATH}
  COMPONENT dev)
install(FILES ${DEFINES_FILE} DESTINATION include/eq COMPONENT dev)

set(DOXYGEN_EXTRA_INPUT "${CMAKE_INSTALL_PREFIX}/include/seq ${CMAKE_INSTALL_PREFIX}/share/Equalizer/examples")
set(DOXYGEN_EXTRA_EXCLUDE "${CMAKE_INSTALL_PREFIX}/include/eq/admin {CMAKE_INSTALL_PREFIX}/include/GL")

include(CPackConfig)

set(DOXYGEN_MAINPAGE_MD README.md)
set(DOXYGEN_EXTRA_INPUT
 "${PROJECT_SOURCE_DIR}/README.md ${CMAKE_INSTALL_PREFIX}/include/seq")
include(DoxygenRule) # must be after all targets

set(CTEST_CUSTOM_WARNING_EXCEPTION ${CTEST_CUSTOM_WARNING_EXCEPTION}
  # http://public.kitware.com/Bug/view.php?id=10179
  "ld: warning: duplicate dylib")
include(CTest)
