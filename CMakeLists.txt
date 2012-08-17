
# Copyright (c) 2010 Daniel Pfeifer <daniel@pfeifer-mail.de>
#               2010-2012 Stefan Eilemann <eile@eyescale.ch>

cmake_minimum_required(VERSION 2.6.4 FATAL_ERROR)
project(Equalizer)

option(EQUALIZER_USE_MAGELLAN "Test for SpaceMouse support" ON)
option(EQUALIZER_BUILD_EXAMPLES "Build Equalizer examples." ON)
option(EQUALIZER_INSTALL_SERVER_HEADERS "Install Equalizer server includes." OFF)
option(EQUALIZER_BUILD_2_0_API
  "Enable for pure 2.0 API (breaks compatibility with 1.x API)" OFF)

mark_as_advanced(EQUALIZER_INSTALL_SERVER_HEADERS)

list(APPEND CMAKE_MODULE_PATH ${Equalizer_SOURCE_DIR}/CMake)

include(revision)

set(RELEASE_VERSION OFF) # OFF or 'Mm0' ABI version
set(LAST_RELEASE 1.3.7)
set(VERSION_MAJOR "1")
set(VERSION_MINOR "5")
set(VERSION_PATCH "0")

set(CO_VERSION_MAJOR "0")
set(CO_VERSION_MINOR "7")
set(CO_VERSION_PATCH "0")
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
include(GitTargets)
include(configure)

include_directories(BEFORE ${CMAKE_SOURCE_DIR}/libs)

include(UpdateFile)
include(TestBigEndian)
test_big_endian(EQ_BIG_ENDIAN)

set(EQ_FEATURES)
if(MSVC)
  set(EQ_FEATURES "${EQ_FEATURES} WGL")
  include(UseGnuWin32)
endif(MSVC)

set(PTHREAD_LIBRARIES pthread)

if(LUNCHBOX_USE_OPENMP)
  find_package(OpenMP REQUIRED)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
endif()

if(HWLOC_FOUND)
  set(EQ_FEATURES "${EQ_FEATURES} HWLOC")
endif()

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

  if(EQUALIZER_OSX_ARCHITECTURES MATCHES "64")
    set(CUDA_FOUND 0)
    message(STATUS "  Disable CUDA due to missing 64 bit libraries")
  else()
    set(CUDA_64_BIT_DEVICE_CODE OFF) 
  endif()
endif(APPLE)

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
add_subdirectory(libs)
if(EXISTS "${CMAKE_SOURCE_DIR}/auxiliary" AND
   IS_DIRECTORY "${CMAKE_SOURCE_DIR}/auxiliary")
  add_subdirectory(auxiliary)
endif()
add_subdirectory(doc) # must be after all targets

find_program(LINKS_EXECUTABLE links)
if(LINKS_EXECUTABLE)
  add_custom_command(OUTPUT ${CMAKE_SOURCE_DIR}/RELNOTES.txt
    COMMAND ${LINKS_EXECUTABLE} -dump libs/RelNotes.dox > RELNOTES.txt
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    DEPENDS ${CMAKE_SOURCE_DIR}/libs/RelNotes.dox)
  add_custom_target(RELNOTES.txt ALL DEPENDS ${CMAKE_SOURCE_DIR}/RELNOTES.txt)
endif()

find_package(PythonInterp)
if(PYTHON_EXECUTABLE)
  add_custom_command(OUTPUT ${CMAKE_SOURCE_DIR}/README.rst
    COMMAND ${PYTHON_EXECUTABLE} CMake/html2rst.py libs/RelNotes.dox > README.rst
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    DEPENDS ${CMAKE_SOURCE_DIR}/libs/RelNotes.dox)
  add_custom_target(README.rst ALL DEPENDS ${CMAKE_SOURCE_DIR}/README.rst)
endif()

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
