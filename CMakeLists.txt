# Copyright (c) 2010 Daniel Pfeifer <daniel@pfeifer-mail.de>
#               2010 Stefan Eilemann <eile@eyescale.ch>

cmake_minimum_required(VERSION 2.8 FATAL_ERROR)
project(Equalizer)

list(APPEND CMAKE_MODULE_PATH ${Equalizer_SOURCE_DIR}/CMake)

set(VERSION_MAJOR "0")
set(VERSION_MINOR "9")
set(VERSION_PATCH "2")
set(VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH})

enable_testing()

IF(NOT CMAKE_BUILD_TYPE)
   SET(CMAKE_BUILD_TYPE Debug)
ENDIF(NOT CMAKE_BUILD_TYPE)

option(EQUALIZER_PREFER_AGL "Prefer AGL over GLX on Mac OS X" ON)
option(EQUALIZER_FORCE_OPENMP "Force usage of buggy OpenMP implementation" OFF)

if(CMAKE_SYSTEM_NAME MATCHES "Linux")
  set(LINUX TRUE)
endif(CMAKE_SYSTEM_NAME MATCHES "Linux")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${Equalizer_BINARY_DIR}/bin)
set(EQ_FEATURES)

set(EQ_INCLUDE_DIR ${Equalizer_BINARY_DIR}/include)
file(MAKE_DIRECTORY ${EQ_INCLUDE_DIR})
include_directories(BEFORE ${EQ_INCLUDE_DIR})

if(MSVC)
  add_definitions(
    /D_CRT_SECURE_NO_WARNINGS
    /wd4244 # conversion from X to Y, possible loss of data
    /wd4800 # forcing value to bool 'true' or 'false' (performance warning)
    )
  # http://www.ogre3d.org/forums/viewtopic.php?f=2&t=60015&start=0
  set(CMAKE_CXX_FLAGS "/DWIN32 /D_WINDOWS /W3 /Zm500 /EHsc /GR /WX")
endif(MSVC)

if(WIN32)
  set(EQ_FEATURES "${EQ_FEATURES} WGL")
  set(Boost_USE_STATIC_LIBS ON)
endif(WIN32)
set(Boost_USE_MULTITHREADED ON)
set(Boost_ADDITIONAL_VERSIONS "1.42" "1.43" "1.44")
find_package(Boost COMPONENTS system regex date_time)
if(Boost_FOUND)
  add_definitions(-DBOOST_ALL_NO_LIB)
  link_directories(${Boost_LIBRARY_DIRS})
  include_directories(AFTER SYSTEM ${Boost_INCLUDE_DIRS})
  set(EQ_FEATURES "${EQ_FEATURES} RSP")
endif(Boost_FOUND)

find_package(OpenMP)
if(OPENMP_FOUND)
  if(CMAKE_COMPILER_IS_GNUCXX)
    _Boost_COMPILER_DUMPVERSION(GCC_COMPILER_VERSION)
  endif(CMAKE_COMPILER_IS_GNUCXX)
  if(CMAKE_COMPILER_IS_GNUCXX AND GCC_COMPILER_VERSION LESS 43 AND NOT EQUALIZER_FORCE_OPENMP)
    message(STATUS "  Disable OpenMP due to bug in gcc ${GCC_COMPILER_VERSION}")
  else(CMAKE_COMPILER_IS_GNUCXX AND GCC_COMPILER_VERSION LESS 43 AND NOT EQUALIZER_FORCE_OPENMP)
    set(EQ_OPENMP_USED 1)
    set(EQ_FEATURES "${EQ_FEATURES} OpenMP")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} ${OpenMP_CXX_FLAGS}")
  endif(CMAKE_COMPILER_IS_GNUCXX AND GCC_COMPILER_VERSION LESS 43 AND NOT EQUALIZER_FORCE_OPENMP)
endif(OPENMP_FOUND)

find_package(OpenGL REQUIRED)
include_directories(SYSTEM ${OPENGL_INCLUDE_DIR})

if(APPLE)
  # WAR otherwise MacPorts X11 (/opt/local) is preferred
  LIST(REMOVE_ITEM CMAKE_SYSTEM_LIBRARY_PATH /opt/local/lib)
  LIST(REMOVE_ITEM CMAKE_SYSTEM_PREFIX_PATH /opt/local)
endif(APPLE)

find_package(X11)

if(APPLE)
  LIST(APPEND CMAKE_SYSTEM_LIBRARY_PATH /opt/local/lib)
  LIST(APPEND CMAKE_SYSTEM_PREFIX_PATH /opt/local)
endif(APPLE)

if(X11_FOUND)
  if(APPLE)
    set(CMAKE_INCLUDE_SYSTEM_FLAG_CXX "-isystem ") # Disabled by GNU.cmake!?
    if(EXISTS /Developer/SDKs/MacOSX10.6.sdk)
      if(EQUALIZER_PREFER_AGL)
        set(EQ_AGL_USED 1)
      else(EQUALIZER_PREFER_AGL)
        set(EQ_GLX_USED 1)
      endif(EQUALIZER_PREFER_AGL)
    else(EXISTS /Developer/SDKs/MacOSX10.6.sdk)
      set(EQ_AGL_USED 1)
      set(EQ_GLX_USED 1)
    endif(EXISTS /Developer/SDKs/MacOSX10.6.sdk)
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

find_package(CUDA)

if(WIN32)
  set(CUDA_FOUND 0)
  message(STATUS "  Disable CUDA, not yet supported on Windows")
endif()

if(CMAKE_COMPILER_IS_GNUCXX)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Winvalid-pch -Wnon-virtual-dtor -Wsign-promo -Wshadow -Winit-self -Wno-unknown-pragmas -Wno-unused-parameter -Wno-write-strings")
  set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Wuninitialized")
  if(NOT WIN32 AND NOT XCODE_VERSION)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror")
  endif()
endif(CMAKE_COMPILER_IS_GNUCXX)

if(LINUX)
  add_definitions(-fPIC) # 64bit Linux wants this
endif(LINUX)

if(APPLE)
  link_directories(/usr/X11R6/lib)
  include_directories(SYSTEM /usr/X11R6/include)
  add_definitions(-DDarwin)
  if(EQ_AGL_USED)
    if(_CMAKE_OSX_MACHINE MATCHES "ppc")
      set(CMAKE_OSX_ARCHITECTURES "ppc")
    else()
      set(CMAKE_OSX_ARCHITECTURES "i386")
    endif()
    set(CUDA_64_BIT_DEVICE_CODE OFF) 
  else(EQ_AGL_USED)
    if(_CMAKE_OSX_MACHINE MATCHES "64" AND CUDA_FOUND)
      set(CUDA_FOUND 0)
      message(STATUS "  Disable CUDA due to missing 64 bit libraries")
    endif()
  endif(EQ_AGL_USED)
endif(APPLE)

if(CUDA_FOUND)
  include_directories(SYSTEM ${CUDA_INCLUDE_DIRS})
  set(CUDA_PROPAGATE_HOST_FLAGS OFF)
  set(EQ_FEATURES "${EQ_FEATURES} CUDA")
endif(CUDA_FOUND)


include(${Equalizer_SOURCE_DIR}/externals/UseGLEW.cmake)
include(${Equalizer_SOURCE_DIR}/externals/UseVMMLib.cmake)
include(${Equalizer_SOURCE_DIR}/externals/UseGnuWin32.cmake)
include(${Equalizer_SOURCE_DIR}/externals/UsePthreads.cmake)
set(PTHREAD_LIBRARIES pthread)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DEQ_BUILD_DIR=\"${CMAKE_BINARY_DIR}/\"")


add_subdirectory(examples)
add_subdirectory(libs)
add_subdirectory(tests)
add_subdirectory(tools)

IF(EXISTS "${CMAKE_SOURCE_DIR}/auxiliary" AND IS_DIRECTORY "${CMAKE_SOURCE_DIR}/auxiliary")
  add_subdirectory(auxiliary)
ENDIF()

IF(EXISTS "${CMAKE_SOURCE_DIR}/man/man3")
  install(DIRECTORY man/man3 DESTINATION man COMPONENT man
          PATTERN "*_docs_*" EXCLUDE)
ENDIF(EXISTS "${CMAKE_SOURCE_DIR}/man/man3")


if(APPLE AND CMAKE_OSX_ARCHITECTURES)
  message(STATUS "Supported Features:${EQ_FEATURES} [${CMAKE_OSX_ARCHITECTURES}]")
else()
  message(STATUS "Supported Features:${EQ_FEATURES}")
endif()

#info: http://www.itk.org/Wiki/CMake:Component_Install_With_CPack

set(CPACK_PACKAGE_NAME "Equalizer")
set(CPACK_PACKAGE_VENDOR "http://www.eyescale.ch")
set(CPACK_PACKAGE_CONTACT "Stefan Eilemann <eile@eyescale.ch>")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Parallel Rendering Framework")
set(CPACK_PACKAGE_VERSION_MAJOR ${VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${VERSION_PATCH})
set(CPACK_RESOURCE_FILE_LICENSE ${Equalizer_SOURCE_DIR}/LICENSE.txt)
set(CPACK_RESOURCE_FILE_README ${Equalizer_SOURCE_DIR}/RELNOTES.txt)

set(CPACK_COMPONENTS_ALL lib dev man apps examples tools data vmmlib)

set(CPACK_COMPONENT_LIB_DISPLAY_NAME "Core Libraries")
set(CPACK_COMPONENT_LIB_DESCRIPTION "Equalizer and Collage Runtime Libraries")

set(CPACK_COMPONENT_DEV_DISPLAY_NAME "Headers")
set(CPACK_COMPONENT_DEV_DESCRIPTION "Header Files for Development")
set(CPACK_COMPONENT_DEV_DEPENDS vmmlib lib)

set(CPACK_COMPONENT_MAN_DISPLAY_NAME "Man Pages")
set(CPACK_COMPONENT_MAN_DESCRIPTION "Manual Pages")
set(CPACK_COMPONENT_MAN_DEPENDS dev)

set(CPACK_COMPONENT_APPS_DISPLAY_NAME "Example Applications")
set(CPACK_COMPONENT_APPS_DESCRIPTION "Example programs build with Equalizer")
set(CPACK_COMPONENT_APPS_DEPENDS lib data)

set(CPACK_COMPONENT_EXAMPLES_DISPLAY_NAME "Examples Source Code")
set(CPACK_COMPONENT_EXAMPLES_DESCRIPTION "Source code of example programs")
set(CPACK_COMPONENT_EXAMPLES_DEPENDS dev data)

set(CPACK_COMPONENT_TOOLS_DISPLAY_NAME "Tools")
set(CPACK_COMPONENT_TOOLS_DESCRIPTION "Utility programs around Equalizer")
set(CPACK_COMPONENT_TOOLS_DEPENDS lib)

set(CPACK_COMPONENT_DATA_DISPLAY_NAME "Data files")
set(CPACK_COMPONENT_DATA_DESCRIPTION "Example configuration files and data sets")

set(CPACK_COMPONENT_VMMLIB_DISPLAY_NAME "VMMLib header files")
set(CPACK_COMPONENT_VMMLIB_DESCRIPTION
  "vmmlib is a vector and matrix math library implemented using C++ templates, thus making it very easy to integrate into other libraries and programs.")

set(CPACK_DEBIAN_PACKAGE_DEPENDS "libboost-system-dev, libboost-regex-dev, libboost-date-time-dev, bison, flex, libx11-dev")

if(MSVC)
  set(CPACK_GENERATOR NSIS)
  set(CPACK_NSIS_PACKAGE_NAME "Equalizer")
  set(CPACK_NSIS_DISPLAY_NAME "Equalizer Parallel Rendering Framework")
endif(MSVC)

include(InstallRequiredSystemLibraries)
include(CPack)
include(copyScript)


SET(CTEST_CUSTOM_WARNING_EXCEPTION
  ${CTEST_CUSTOM_WARNING_EXCEPTION}

  # http://public.kitware.com/Bug/view.php?id=10179
  "ld: warning: duplicate dylib"
)
include(CTest)
