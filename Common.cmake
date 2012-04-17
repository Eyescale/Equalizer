
# Common settings
enable_testing()
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
list(APPEND CMAKE_PREFIX_PATH ${SystemDrive}:/cygwin/bin)
if(CMAKE_VERSION VERSION_LESS 2.8)
  list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/CMake/2.8)
endif()

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug)
endif(NOT CMAKE_BUILD_TYPE)

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT AND NOT MSVC)
  set(CMAKE_INSTALL_PREFIX "/usr" CACHE PATH
    "${CMAKE_PROJECT_NAME} install prefix" FORCE)
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

set(OUTPUT_INCLUDE_DIR ${CMAKE_BINARY_DIR}/include)
file(MAKE_DIRECTORY ${OUTPUT_INCLUDE_DIR})
include_directories(BEFORE ${CMAKE_SOURCE_DIR} ${OUTPUT_INCLUDE_DIR})

# Boost settings
if(MSVC)
  option(Boost_USE_STATIC_LIBS "Use boost static libs" ON)
endif()
if(BOOST_ROOT)
  set(Boost_NO_SYSTEM_PATHS TRUE)
endif()
add_definitions(-DBOOST_ALL_NO_LIB) # Don't use 'pragma lib' on Windows

# Compiler setting
if(CMAKE_COMPILER_IS_GNUCXX)
  include(EqCompilerVersion)
  EQ_COMPILER_DUMPVERSION(GCC_COMPILER_VERSION)
  if(GCC_COMPILER_VERSION VERSION_LESS 4.1)
    message(ERROR "GCC 4.1 or later required, found ${GCC_COMPILER_VERSION}")
  endif()
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Winvalid-pch -Wnon-virtual-dtor -Wsign-promo -Wshadow -Winit-self -Wno-unknown-pragmas -Wno-unused-parameter -Wno-write-strings")
  set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Wuninitialized")
  if(NOT WIN32 AND NOT XCODE_VERSION AND NOT RELEASE_VERSION)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror")
  endif()
endif(CMAKE_COMPILER_IS_GNUCXX)

if(MSVC)
  add_definitions(
    /D_CRT_SECURE_NO_WARNINGS
    /D_SCL_SECURE_NO_WARNINGS
    /wd4244 # conversion from X to Y, possible loss of data
    /wd4800 # forcing value to bool 'true' or 'false' (performance warning)
    )

  # http://www.ogre3d.org/forums/viewtopic.php?f=2&t=60015&start=0
  if(RELEASE_VERSION)
    set(CMAKE_CXX_FLAGS "/DWIN32 /D_WINDOWS /W3 /Zm500 /EHsc /GR")
  else()
    set(CMAKE_CXX_FLAGS "/DWIN32 /D_WINDOWS /W3 /Zm500 /EHsc /GR /WX")
  endif()
endif(MSVC)

if(CMAKE_SYSTEM_NAME MATCHES "Linux")
  set(LINUX TRUE)
  add_definitions(-fPIC)

  if(REDHAT AND CMAKE_SYSTEM_PROCESSOR MATCHES "64$")
    set(LIB_SUFFIX 64 CACHE STRING "Library directory suffix")
  endif()
  set(LIBRARY_DIR lib${LIB_SUFFIX})
endif()

if(APPLE)
  if(_CMAKE_OSX_MACHINE MATCHES "ppc")
    set(OSX_ARCHITECTURES "ppc;ppc64" CACHE STRING "Build architectures")
  else()
    set(OSX_ARCHITECTURES "i386;x86_64" CACHE STRING "Build architectures")
  endif()
  set(CMAKE_OSX_ARCHITECTURES ${OSX_ARCHITECTURES})
  set(CMAKE_INCLUDE_SYSTEM_FLAG_C "-isystem ")
  set(CMAKE_INCLUDE_SYSTEM_FLAG_CXX "-isystem ")
  message(STATUS "Building for ${CMAKE_OSX_ARCHITECTURES}")
endif(APPLE)

