
# Common settings
enable_testing()
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
list(APPEND CMAKE_PREFIX_PATH ${SystemDrive}:/cygwin/bin)
if(CMAKE_VERSION VERSION_LESS 2.8)
  list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/CMake/2.8)
endif()
if(CMAKE_VERSION VERSION_LESS 2.8.3)
  list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/CMake/2.8.3)
endif()

if(NOT CMAKE_BUILD_TYPE)
  if(RELEASE_VERSION)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
  else()
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
  endif()
endif(NOT CMAKE_BUILD_TYPE)

set(VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH})
string(TOUPPER ${CMAKE_PROJECT_NAME} UPPER_PROJECT_NAME)
add_definitions(-D${UPPER_PROJECT_NAME}_VERSION=${VERSION})
if(NOT VERSION_ABI)
  if(RELEASE_VERSION)
    set(VERSION_ABI ${RELEASE_VERSION})
  else()
    if(VERSION_MAJOR VERSION_GREATER 0)
      set(VERSION_ABI ${VERSION_MAJOR}${VERSION_MINOR}${VERSION_PATCH})
    else()
      set(VERSION_ABI ${VERSION_MINOR}${VERSION_PATCH})
    endif()
  endif()
endif()

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

string(REGEX REPLACE ".*\\/(share\\/.*)" "\\1/Modules" CMAKE_MODULE_INSTALL_PATH
  ${CMAKE_ROOT})

# Boost settings
if(MSVC)
  option(Boost_USE_STATIC_LIBS "Use boost static libs" ON)
endif()
if(BOOST_ROOT)
  set(Boost_NO_SYSTEM_PATHS TRUE)
endif()
add_definitions(-DBOOST_ALL_NO_LIB) # Don't use 'pragma lib' on Windows

# Compiler settings
if(CMAKE_CXX_COMPILER_ID STREQUAL "XL")
  set(CMAKE_COMPILER_IS_XLCXX ON)
endif()

if(CMAKE_COMPILER_IS_GNUCXX)
  include(CompilerVersion)
  COMPILER_DUMPVERSION(GCC_COMPILER_VERSION)
  if(GCC_COMPILER_VERSION VERSION_LESS 4.1)
    message(ERROR "GCC 4.1 or later required, found ${GCC_COMPILER_VERSION}")
  endif()
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Winvalid-pch -Wnon-virtual-dtor -Wsign-promo -Wshadow -Winit-self -Wno-unknown-pragmas -Wno-unused-parameter")
  set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Wuninitialized")
  set(CMAKE_CXX_FLAGS_DEBUG
    "${CMAKE_CXX_FLAGS_DEBUG} -fprofile-arcs -ftest-coverage")
  if(NOT WIN32 AND NOT XCODE_VERSION AND NOT RELEASE_VERSION)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror")
  endif()
elseif(CMAKE_COMPILER_IS_XLCXX)
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -qnoopt")
  set(CMAKE_CC_FLAGS_DEBUG "${CMAKE_CC_FLAGS_DEBUG} -O0 -qnoopt")
endif()

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
elseif(EXISTS ${CMAKE_SOURCE_DIR}/CMake/${CMAKE_PROJECT_NAME}.in.spec)
  configure_file(${CMAKE_SOURCE_DIR}/CMake/${CMAKE_PROJECT_NAME}.in.spec 
    ${CMAKE_SOURCE_DIR}/CMake/${CMAKE_PROJECT_NAME}.spec @ONLY)
endif()

if(CMAKE_SYSTEM_NAME MATCHES "Linux")
  set(LINUX TRUE)

  if(REDHAT AND CMAKE_SYSTEM_PROCESSOR MATCHES "64$")
    set(LIB_SUFFIX 64 CACHE STRING "Library directory suffix")
  endif()
  if(CMAKE_SYSTEM_PROCESSOR MATCHES "ppc")
    set(LINUX_PPC 1)
  else()
    add_definitions(-fPIC)
  endif()
endif()
set(LIBRARY_DIR lib${LIB_SUFFIX})

if(APPLE)
  if(_CMAKE_OSX_MACHINE MATCHES "ppc")
    set(OSX_ARCHITECTURES "ppc;ppc64" CACHE STRING "Build architectures")
  else()
    set(OSX_ARCHITECTURES "i386;x86_64" CACHE STRING "Build architectures")
  endif()
  set(CMAKE_OSX_ARCHITECTURES ${OSX_ARCHITECTURES})
  set(CMAKE_INCLUDE_SYSTEM_FLAG_C "-isystem ")
  set(CMAKE_INCLUDE_SYSTEM_FLAG_CXX "-isystem ")
  if (NOT CMAKE_INSTALL_NAME_DIR)
    set(CMAKE_INSTALL_NAME_DIR "${CMAKE_INSTALL_PREFIX}/lib")
  endif (NOT CMAKE_INSTALL_NAME_DIR)
  message(STATUS
    "Building ${CMAKE_PROJECT_NAME} for ${CMAKE_OSX_ARCHITECTURES}")
endif(APPLE)

# hooks to gather all targets (libaries & executables)
set(ALL_DEP_TARGETS "")
macro(add_executable _target)
  _add_executable(${_target} ${ARGN})
  set_property(GLOBAL APPEND PROPERTY ALL_DEP_TARGETS ${_target})
endmacro()
macro(add_library _target)
  _add_library(${_target} ${ARGN})
  set_property(GLOBAL APPEND PROPERTY ALL_DEP_TARGETS ${_target})
endmacro()
