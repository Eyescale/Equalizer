
# Copyright (c) 2010 Daniel Pfeifer <daniel@pfeifer-mail.de>
#               2010-2012 Stefan Eilemann <eile@eyescale.ch>

cmake_minimum_required(VERSION 2.6.4 FATAL_ERROR)
project(Equalizer)

option(EQUALIZER_USE_OPENMP "Test for OpenMP support" ON)
option(EQUALIZER_FORCE_OPENMP "Force usage of buggy OpenMP implementation" OFF)
option(EQUALIZER_USE_GPUSD "Test for GPU-SD support" ON)
option(EQUALIZER_USE_HWLOC "Test for hwloc support" ON)
option(EQUALIZER_USE_MAGELLAN "Test for SpaceMouse support" ON)
option(EQUALIZER_BUILD_EXAMPLES "Build Equalizer examples." ON)
option(EQUALIZER_INSTALL_SERVER_HEADERS "Install Equalizer server includes." OFF)
option(EQUALIZER_BUILD_2_0_API
  "Enable for pure 2.0 API (breaks compatibility with 1.x API)" OFF)

mark_as_advanced(EQUALIZER_INSTALL_SERVER_HEADERS)
mark_as_advanced(EQUALIZER_FORCE_OPENMP)

list(APPEND CMAKE_MODULE_PATH ${Equalizer_SOURCE_DIR}/CMake)

include(revision)

set(RELEASE_VERSION OFF) # OFF or 'Mm0' ABI version
set(VERSION_MAJOR "1")
set(VERSION_MINOR "3")
set(VERSION_PATCH "5")
set(VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH})

set(CO_VERSION_MAJOR "0")
set(CO_VERSION_MINOR "5")
set(CO_VERSION_PATCH "5")
set(CO_VERSION ${CO_VERSION_MAJOR}.${CO_VERSION_MINOR}.${CO_VERSION_PATCH})

set(EQ_OSX_VERSION 10.6)
if(RELEASE_VERSION)
  set(EQ_DEFINITIONS -DEQ_RELEASE)
  set(CMAKE_OSX_SYSROOT /Developer/SDKs/MacOSX${EQ_OSX_VERSION}.sdk
      CACHE PATH "" FORCE)
  set(VERSION_ABI ${RELEASE_VERSION})
else()
  set(VERSION_ABI ${VERSION_MAJOR}${VERSION_MINOR}${VERSION_PATCH})
endif()

include(Common)
include_directories(BEFORE ${CMAKE_SOURCE_DIR}/libs)

if(EQUALIZER_USE_GPUSD)
  find_package(gpusd 1.1.2)
endif()

set(EQ_FEATURES)

find_package(Doxygen)
if(DOXYGEN_FOUND)
  set (ALL_TARGETS "")
  macro(add_executable _target)
    _add_executable (${_target} ${ARGN})
    set_property (GLOBAL APPEND PROPERTY ALL_TARGETS ${_target})
  endmacro()
endif()

include(configure)
include(UpdateFile)
include(TestBigEndian)
test_big_endian(EQ_BIG_ENDIAN)

if(MSVC)
  set(EQ_FEATURES "${EQ_FEATURES} WGL")
  include(UseGnuWin32)
endif(MSVC)

find_package(Boost 1.36 REQUIRED system regex date_time serialization)
if(Boost_VERSION GREATER 103999)
  set(CO_USE_BOOST_SERIALIZATION 1)
endif()
link_directories(${Boost_LIBRARY_DIRS})
include_directories(AFTER SYSTEM ${Boost_INCLUDE_DIRS})

set(PTHREAD_LIBRARIES pthread)
link_directories(${LUNCHBOX_LIBRARY_DIRS})

if(EQUALIZER_USE_OPENMP)
  find_package(OpenMP)
endif()
if(OPENMP_FOUND)
  if(CMAKE_COMPILER_IS_GNUCXX AND GCC_COMPILER_VERSION VERSION_LESS 4.3 AND
      NOT EQUALIZER_FORCE_OPENMP)
    message(STATUS "Disable OpenMP due to bug in gcc ${GCC_COMPILER_VERSION}")
  else()
    set(CO_OPENMP_USED 1)
    set(EQ_FEATURES "${EQ_FEATURES} OpenMP")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
  endif()
endif(OPENMP_FOUND)

find_package(OFED)
if(OFED_FOUND)
  set(EQ_FEATURES "${EQ_FEATURES} RDMA")
  include_directories(SYSTEM ${OFED_INCLUDE_DIRS})
endif()

find_package(UDT)
if(UDT_FOUND)
  set(EQ_FEATURES "${EQ_FEATURES} UDT")
  include_directories(SYSTEM ${UDT_INCLUDE_DIRS})
endif()

if(EQUALIZER_USE_HWLOC)
  find_package(HWLOC)
endif()
if(HWLOC_FOUND)
  include_directories(SYSTEM ${HWLOC_INCLUDE_DIR})
  set(EQ_FEATURES "${EQ_FEATURES} HWLOC")
endif()

if(APPLE)
  # WAR otherwise MacPorts X11 (/opt/local) is preferred
  LIST(REMOVE_ITEM CMAKE_SYSTEM_LIBRARY_PATH /opt/local/lib)
  LIST(REMOVE_ITEM CMAKE_SYSTEM_PREFIX_PATH /opt/local)
endif()

find_package(X11)

if(APPLE)
  LIST(APPEND CMAKE_SYSTEM_LIBRARY_PATH /opt/local/lib)
  LIST(APPEND CMAKE_SYSTEM_PREFIX_PATH /opt/local)
endif(APPLE)

if(X11_FOUND)
  if(APPLE)
    set(CMAKE_INCLUDE_SYSTEM_FLAG_CXX "-isystem ") # Disabled by GNU.cmake!?
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mmacosx-version-min=${EQ_OSX_VERSION}" )
    set(EQ_AGL_USED 1)
    set(EQ_GLX_USED 1)
  else(APPLE)
    set(EQ_GLX_USED 1)
  endif(APPLE)
endif(X11_FOUND)

if(EQ_GLX_USED)
  include_directories(SYSTEM ${X11_INCLUDE_DIR})
  set(EQ_FEATURES "${EQ_FEATURES} GLX")
endif(EQ_GLX_USED)
if(EQ_AGL_USED)
  set(EQ_FEATURES "${EQ_FEATURES} AGL")
endif(EQ_AGL_USED)

if(APPLE)
  link_directories(/usr/X11R6/lib)
  include_directories(SYSTEM /usr/X11R6/include)
  add_definitions(-DDarwin)

  if(_CMAKE_OSX_MACHINE MATCHES "ppc")
    set(EQUALIZER_OSX_ARCHITECTURES "ppc;ppc64" CACHE STRING
      "Equalizer build architectures")
  else()
    set(EQUALIZER_OSX_ARCHITECTURES "i386;x86_64" CACHE STRING
      "Equalizer build architectures")
  endif()
  mark_as_advanced(EQUALIZER_OSX_ARCHITECTURES)
  
  set(CMAKE_OSX_ARCHITECTURES "${EQUALIZER_OSX_ARCHITECTURES}")
  message(STATUS "Building Equalizer for ${CMAKE_OSX_ARCHITECTURES}")
  if(EQUALIZER_OSX_ARCHITECTURES MATCHES "64")
    set(CUDA_FOUND 0)
    message(STATUS "  Disable CUDA due to missing 64 bit libraries")
  else()
    set(CUDA_64_BIT_DEVICE_CODE OFF) 
  endif()
endif(APPLE)

set(EQ_DEFINITIONS ${EQ_DEFINITIONS}
    -DEQ_BUILD_DIR="${CMAKE_BINARY_DIR}/" -DEQ_SOURCE_DIR="${CMAKE_SOURCE_DIR}/")

if(EQUALIZER_BUILD_2_0_API)
  set(EQ_DEFINITIONS ${EQ_DEFINITIONS} -DEQ_2_0_API)
  set(EQ_FEATURES "${EQ_FEATURES} APIv2")
else()
  set(EQ_FEATURES "${EQ_FEATURES} APIv1")
endif()
if(IS_ABSOLUTE ${CMAKE_INSTALL_PREFIX})
  set(EQ_DEFINITIONS ${EQ_DEFINITIONS} -DEQ_INSTALL_DIR="${CMAKE_INSTALL_PREFIX}/")
else()
  set(EQ_DEFINITIONS ${EQ_DEFINITIONS} -DEQ_INSTALL_DIR="${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_PREFIX}/")
endif()
add_definitions(${EQ_DEFINITIONS})

add_subdirectory(tools)
if(EQUALIZER_BUILD_EXAMPLES)
  add_subdirectory(examples)
endif()
if(EXISTS "${Equalizer_SOURCE_DIR}/tests/CMakeLists.txt")
  add_subdirectory(tests)
endif()
add_subdirectory(libs)
if(EXISTS "${CMAKE_SOURCE_DIR}/auxiliary" AND
   IS_DIRECTORY "${CMAKE_SOURCE_DIR}/auxiliary")
  add_subdirectory(auxiliary)
endif()
add_subdirectory(doc) # must be after all targets

install(FILES CMake/FindCollage.cmake
  DESTINATION ${CMAKE_MODULE_INSTALL_PATH} COMPONENT codev)
install(FILES CMake/FindEqualizer.cmake
  DESTINATION ${CMAKE_MODULE_INSTALL_PATH} COMPONENT eqdev)
install(FILES CMake/FindGLEW_MX.cmake
  DESTINATION ${CMAKE_MODULE_INSTALL_PATH}/FindEqualizer COMPONENT eqdev)

if(LUNCHBOX_USE_DNSSD)
  set(EQ_FEATURES "${EQ_FEATURES} zeroconf")
endif()
if(GPUSD_FOUND)
  set(EQ_FEATURES "${EQ_FEATURES} GPUSD")
endif()
if(CUDA_FOUND)
  set(EQ_FEATURES "${EQ_FEATURES} CUDA")
endif()

if(MSVC)
  message(STATUS "Configured with${EQ_FEATURES}")
else()
  message(STATUS "Configured with ${CMAKE_BUILD_TYPE}${EQ_FEATURES}")
endif()

include(CPackConfig)
include(copyScript)

set(CTEST_CUSTOM_WARNING_EXCEPTION ${CTEST_CUSTOM_WARNING_EXCEPTION}
  # http://public.kitware.com/Bug/view.php?id=10179
  "ld: warning: duplicate dylib")
include(CTest)
