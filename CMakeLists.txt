
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

if(APPLE)
  if(CMAKE_OSX_ARCHITECTURES MATCHES "i386")
    set(EQ_AGL_USED 1)
  endif()
  # WAR otherwise MacPorts X11 (/opt/local) is preferred
  list(REMOVE_ITEM CMAKE_SYSTEM_LIBRARY_PATH /opt/local/lib /usr/local/lib)
  list(REMOVE_ITEM CMAKE_SYSTEM_PREFIX_PATH /opt/local /usr/local)
  link_directories(/opt/X11/lib /usr/X11R6/lib)
  include_directories(SYSTEM /opt/X11/include /usr/X11R6/include)
endif()

common_find_package(Boost REQUIRED COMPONENTS
  program_options filesystem system thread)
common_find_package(Collage REQUIRED)
common_find_package(Deflect)
common_find_package(GLStats)
common_find_package(hwloc)
common_find_package(hwsd)
common_find_package(Lunchbox REQUIRED)
common_find_package(MAGELLAN)
common_find_package(OpenCV)
common_find_package(OpenGL REQUIRED)
if(EQUALIZER_USE_OSG)
  common_find_package(OpenSceneGraph SYSTEM COMPONENTS osgDB osgUtil)
endif()
common_find_package(Pression REQUIRED)
common_find_package(Qt5Core)
common_find_package(Qt5Gui)
common_find_package(Qt5Widgets)
common_find_package(vmmlib REQUIRED)
common_find_package(VRPN)
common_find_package(X11 SYSTEM)
if(APPLE) # tweak window systems before finding glew
  if(EQ_AGL_USED)
    common_find_package_disable(X11)
  else()
    if(EQUALIZER_PREFER_QT AND Qt5Gui_FOUND)
      common_find_package_disable(X11)
    elseif(X11_FOUND)
      common_find_package_disable(Qt5Core Qt5Gui Qt5Widgets)
    endif()
  endif()
endif()
common_find_package(GLEW_MX)

list(APPEND COMMON_FIND_PACKAGE_DEFINES GLEW_MX GLEW_NO_GLU)
if(GLEW_MX_FOUND)
  list(APPEND COMMON_FIND_PACKAGE_DEFINES EQ_FOUND_GLEW_MX)
else()
  list(APPEND COMMON_FIND_PACKAGE_DEFINES GLEW_BUILD EQ_GLEW_INTERNAL)
  if(X11_FOUND AND APPLE)
    list(APPEND COMMON_FIND_PACKAGE_DEFINES GLEW_APPLE_GLX)
  endif()
endif()

if(WIN32)
  list(APPEND COMMON_FIND_PACKAGE_DEFINES WGL) # deprecated
  list(APPEND COMMON_FIND_PACKAGE_DEFINES EQ_WGL_USED)
endif()

if(Qt5Gui_FOUND AND Qt5Gui_VERSION VERSION_GREATER 5.0)
  set(EQ_QT_USED 1)
endif()

if(X11_FOUND)
  set(EQ_GLX_USED 1)
endif()

include(EqGLLibraries)

if(EQ_GLX_USED)
  list(APPEND COMMON_FIND_PACKAGE_DEFINES GLX) # deprecated
  list(APPEND COMMON_FIND_PACKAGE_DEFINES EQ_GLX_USED)
  if(MAGELLAN_FOUND AND NOT APPLE)
    list(APPEND COMMON_FIND_PACKAGE_DEFINES EQUALIZER_USE_MAGELLAN_GLX)
  endif()
elseif(EQ_AGL_USED)
  list(APPEND COMMON_FIND_PACKAGE_DEFINES AGL) # deprecated
  list(APPEND COMMON_FIND_PACKAGE_DEFINES EQ_AGL_USED)
endif()

if(EQ_QT_USED)
  list(APPEND COMMON_FIND_PACKAGE_DEFINES EQ_QT_USED)
endif()

if(NOT EQUALIZER_USE_OSG)
  set(OPENSCENEGRAPH_FOUND)
endif()

if(NOT EQUALIZER_USE_HWLOC)
  common_find_package_disable(hwloc)
  set(HWLOC_GL_FOUND)
endif()
if(HWLOC_GL_FOUND)
  list(APPEND COMMON_FIND_PACKAGE_DEFINES EQUALIZER_USE_HWLOC_GL)
endif()

common_find_package_post()

if(APPLE)
  LIST(APPEND CMAKE_SYSTEM_LIBRARY_PATH /opt/local/lib /usr/local/lib)
  LIST(APPEND CMAKE_SYSTEM_PREFIX_PATH /opt/local /usr/local)
endif()

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
