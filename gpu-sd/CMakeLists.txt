
cmake_minimum_required(VERSION 2.6 FATAL_ERROR)
project(gpusd)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/CMake)
list(APPEND CMAKE_PREFIX_PATH ${SystemDrive}:/cygwin/bin)

set(VERSION_MAJOR "1")
set(VERSION_MINOR "1")
set(VERSION_PATCH "1")
set(VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH})

enable_testing()

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug)
endif(NOT CMAKE_BUILD_TYPE)

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT AND NOT MSVC)
  if(APPLE)
    set(CMAKE_INSTALL_PREFIX "/usr/local" CACHE PATH "Installation prefix" FORCE)
  else()
    set(CMAKE_INSTALL_PREFIX "/usr" CACHE PATH "Installation prefix" FORCE)
  endif()
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(OUTPUT_INCLUDE_DIR ${CMAKE_CURRENT_BINARY_DIR}/libs)
file(MAKE_DIRECTORY ${OUTPUT_INCLUDE_DIR})
include_directories(BEFORE ${CMAKE_CURRENT_SOURCE_DIR}/libs ${OUTPUT_INCLUDE_DIR})

find_package(Boost COMPONENTS program_options) 
find_package(DNSSD)

if(APPLE)
  set(GPUSD_OSX_VERSION 10.6)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mmacosx-version-min=${GPUSD_OSX_VERSION}" )

  # WAR otherwise MacPorts X11 (/opt/local) is preferred
  LIST(REMOVE_ITEM CMAKE_SYSTEM_LIBRARY_PATH /opt/local/lib)
  LIST(REMOVE_ITEM CMAKE_SYSTEM_PREFIX_PATH /opt/local)
endif()

find_package(X11)

if(APPLE)
  LIST(APPEND CMAKE_SYSTEM_LIBRARY_PATH /opt/local/lib)
  LIST(APPEND CMAKE_SYSTEM_PREFIX_PATH /opt/local)
endif(APPLE)

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
  if(_CMAKE_OSX_MACHINE MATCHES "ppc")
    set(GPUSD_OSX_ARCHITECTURES "ppc;ppc64" CACHE STRING
        "OS X build architectures")
  else()
    set(GPUSD_OSX_ARCHITECTURES "i386;x86_64" CACHE STRING
        "OS X build architectures")
  endif()
  mark_as_advanced(GPUSD_OSX_ARCHITECTURES)
  
  set(CMAKE_OSX_ARCHITECTURES "${GPUSD_OSX_ARCHITECTURES}")
  message(STATUS "Building gpu_sd for ${CMAKE_OSX_ARCHITECTURES}")
endif(APPLE)

if(MSVC)
  add_definitions(
    /D_CRT_SECURE_NO_WARNINGS
    /wd4244 # conversion from X to Y, possible loss of data
    /wd4800 # forcing value to bool 'true' or 'false' (performance warning)
    )
  # http://www.ogre3d.org/forums/viewtopic.php?f=2&t=60015&start=0
  set(CMAKE_CXX_FLAGS "/DWIN32 /D_WINDOWS /W3 /Zm500 /EHsc /GR /WX")
endif(MSVC)

add_subdirectory(apps)
add_subdirectory(libs)
add_subdirectory(doc)

install(FILES CMake/FindGPUSD.cmake DESTINATION share/gpu-sd/CMake
        COMPONENT dev)

include(CTest)
include(CPackConfig)
