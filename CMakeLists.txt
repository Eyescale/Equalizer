
# Copyright (c) 2010 Daniel Pfeifer <daniel@pfeifer-mail.de>
#               2010-2013 Stefan Eilemann <eile@eyescale.ch>

cmake_minimum_required(VERSION 2.8 FATAL_ERROR)
project(Equalizer)
list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/CMake)
if(NOT BUILDYARD)
  include(Buildyard)
  return()
endif()

option(EQUALIZER_USE_MAGELLAN "Test for SpaceMouse support" ON)
option(EQUALIZER_BUILD_EXAMPLES "Build Equalizer examples." ON)
option(EQUALIZER_INSTALL_SERVER_HEADERS "Install Equalizer server includes." OFF)
option(EQUALIZER_BUILD_2_0_API
  "Enable for pure 2.0 API (breaks compatibility with 1.x API)" OFF)

mark_as_advanced(EQUALIZER_INSTALL_SERVER_HEADERS)

include(Revision)

set(RELEASE_VERSION OFF) # OFF or 'Mm0' ABI version
set(LAST_RELEASE 1.6.0)
set(VERSION_MAJOR "1")
set(VERSION_MINOR "7")
set(VERSION_PATCH "0")

if(CMAKE_OSX_DEPLOYMENT_TARGET)
  set(EQ_OSX_VERSION ${CMAKE_OSX_DEPLOYMENT_TARGET})
else()
  set(EQ_OSX_VERSION 10.6)
endif()

if(RELEASE_VERSION)
  set(EQ_DEFINITIONS -DEQ_RELEASE)
  set(CMAKE_OSX_SYSROOT /Developer/SDKs/MacOSX${EQ_OSX_VERSION}.sdk
      CACHE PATH "" FORCE)
  set(VERSION_ABI ${RELEASE_VERSION})
else()
  set(VERSION_ABI ${VERSION_MAJOR}${VERSION_MINOR}${VERSION_PATCH})
endif()

include(Common)
include(GitTargets)
include(configure)
include(UpdateFile)
include(TestBigEndian)
test_big_endian(EQ_BIG_ENDIAN)

if(CMAKE_COMPILER_IS_CLANG)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-overloaded-virtual")
endif()

set(EQ_FEATURES)
if(MSVC)
  set(EQ_FEATURES "${EQ_FEATURES} WGL")
  include(UseGnuWin32)
else()
  set(PTHREAD_LIBRARIES pthread)
endif()

if(LUNCHBOX_USE_OPENMP)
  find_package(OpenMP REQUIRED)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
endif()

if(APPLE)
  add_definitions(-DDarwin)
  set(CMAKE_INCLUDE_SYSTEM_FLAG_CXX "-isystem ") # Disabled by GNU.cmake!?
  set(CMAKE_CXX_FLAGS
    "${CMAKE_CXX_FLAGS} -mmacosx-version-min=${EQ_OSX_VERSION}")
endif(APPLE)

if(EQ_GLX_USED)
  include_directories(SYSTEM ${X11_INCLUDE_DIR})
  set(EQ_FEATURES "${EQ_FEATURES} GLX")
endif(EQ_GLX_USED)
if(EQ_AGL_USED)
  set(EQ_FEATURES "${EQ_FEATURES} AGL(32bit)")
endif(EQ_AGL_USED)

list(APPEND EQ_DEFINITIONS
  -DEQ_BUILD_DIR="${CMAKE_BINARY_DIR}/" -DEQ_SOURCE_DIR="${CMAKE_SOURCE_DIR}/")

if(EQUALIZER_BUILD_2_0_API)
  list(APPEND EQ_DEFINITIONS -DEQ_2_0_API)
  set(EQ_FEATURES "${EQ_FEATURES} APIv2")
else()
  set(EQ_FEATURES "${EQ_FEATURES} APIv1")
endif()
if(IS_ABSOLUTE ${CMAKE_INSTALL_PREFIX})
  list(APPEND EQ_DEFINITIONS -DEQ_INSTALL_DIR="${CMAKE_INSTALL_PREFIX}/")
else()
  list(APPEND EQ_DEFINITIONS
    -DEQ_INSTALL_DIR="${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_PREFIX}/")
endif()
add_definitions(${EQ_DEFINITIONS})

add_subdirectory(tools) # before examples, they depend on it
if(EQUALIZER_BUILD_EXAMPLES)
  add_subdirectory(examples)
endif()
if(EXISTS "${Equalizer_SOURCE_DIR}/tests/CMakeLists.txt")
  add_subdirectory(tests)
endif()

include(GLEW/CMakeLists.txt)
add_subdirectory(eq)
add_subdirectory(seq)

if(EXISTS "${CMAKE_SOURCE_DIR}/auxiliary" AND
   IS_DIRECTORY "${CMAKE_SOURCE_DIR}/auxiliary")
  add_subdirectory(auxiliary)
endif()
add_subdirectory(doc)

set(EQUALIZER_DEPENDENT_LIBRARIES Lunchbox Collage)
if(GLEW_MX_FOUND)
  list(APPEND EQUALIZER_DEPENDENT_LIBRARIES GLEW_MX)
endif()

install(FILES CMake/FindGLEW_MX.cmake DESTINATION ${CMAKE_MODULE_INSTALL_PATH}
  COMPONENT dev)

if(HWLOC_GL_FOUND)
  set(EQ_FEATURES "${EQ_FEATURES} HWLOC")
endif()
if(LUNCHBOX_USE_DNSSD)
  set(EQ_FEATURES "${EQ_FEATURES} zeroconf")
endif()
if(HWSD_FOUND)
  set(EQ_FEATURES "${EQ_FEATURES} HWSD")
endif()
if(CUDA_FOUND)
  set(EQ_FEATURES "${EQ_FEATURES} CUDA")
endif()
if(SAGE_FOUND)
  set(EQ_FEATURES "${EQ_FEATURES} SAGE")
endif()
if(DISPLAYCLUSTER_FOUND)
  set(EQ_FEATURES "${EQ_FEATURES} DisplayCluster")
endif()
if(OpenCV_FOUND)
  set(EQ_FEATURES "${EQ_FEATURES} OpenCV")
endif()
if(VRPN_FOUND)
  set(EQ_FEATURES "${EQ_FEATURES} VRPN")
endif()
if(MAGELLAN_FOUND)
  set(EQ_FEATURES "${EQ_FEATURES} SpaceMouse")
endif()

if(MSVC)
  message(STATUS "Configured with${EQ_FEATURES}")
else()
  message(STATUS "Configured with ${CMAKE_BUILD_TYPE}${EQ_FEATURES}")
endif()


include(DoxygenRule) # must be after all targets
include(CPackConfig)
include(PackageConfig)

set(CTEST_CUSTOM_WARNING_EXCEPTION ${CTEST_CUSTOM_WARNING_EXCEPTION}
  # http://public.kitware.com/Bug/view.php?id=10179
  "ld: warning: duplicate dylib")
include(CTest)
