# Copyright (c) 2012-2013 Fabien Delalondre <fabien.delalondre@epfl.ch>
# Adding support for compiler optimization and definition of release configs
# according to chosen compiler 
# Supported compilers are XL, Intel, Clang and GNU

# Compiler settings
if(CMAKE_CXX_COMPILER_ID STREQUAL "XL")
  set(CMAKE_COMPILER_IS_XLCXX ON)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Intel")
  set(CMAKE_COMPILER_IS_INTEL ON)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  set(CMAKE_COMPILER_IS_CLANG ON)
elseif(CMAKE_COMPILER_IS_GNUCXX)
  set(CMAKE_COMPILER_IS_GNUCXX_PURE ON)
endif()

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_COMPILER_IS_CLANG)
  include(${CMAKE_CURRENT_LIST_DIR}/CompilerVersion.cmake)
  COMPILER_DUMPVERSION(GCC_COMPILER_VERSION)
  if(GCC_COMPILER_VERSION VERSION_LESS 4.1)
    message(ERROR "GCC 4.1 or later required, found ${GCC_COMPILER_VERSION}")
  endif()
  set(COMMON_GCC_FLAGS "-Wall -Wextra -Winvalid-pch -Winit-self -Wno-unknown-pragmas -Wno-unused-parameter")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${COMMON_GCC_FLAGS} ")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${COMMON_GCC_FLAGS} -Wnon-virtual-dtor -Wsign-promo")
  if(GCC_COMPILER_VERSION VERSION_GREATER 4.1) # < 4.2 doesn't know -isystem
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wshadow")
  endif()
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-strict-aliasing")
  set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Wuninitialized")
  if(NOT WIN32 AND NOT XCODE_VERSION AND NOT RELEASE_VERSION)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror")
  endif()
  if(CMAKE_COMPILER_IS_CLANG)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Qunused-arguments")
  endif()
elseif(CMAKE_COMPILER_IS_INTEL)
  # default 
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-deprecated -Wno-unknown-pragmas")

  # optimized: Adding xhost to automatically generate instructions set up 
  # to the highest supported compilation host
  set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -xhost")
  set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -xhost")
  end()

elseif(CMAKE_COMPILER_IS_XLCXX)
  # default: Maintain code semantics
  # Fix to link dynamically. On the next pass should add an if statement: if shared ...
  # Setting Release flags and not using default CMake since the default were -O -NDEBUG 
  # By default, set flags for back end since this is the most common use case
  OPTION(XLC_Backend "To compile using XLC compilers on blue gene compute nodes" ON)
  if(XLC_Backend)
    message("Using release configuration for back end compilation (Compute Nodes)")
    set(CMAKE_CXX_FLAGS_RELEASE "-O3 -qtune=qp -qarch=qp -q64 -qstrict -qnohot -qnostaticlink")
    set(CMAKE_C_FLAGS_RELEASE "-O3 -qtune=qp -qarch=qp -q64 -qstrict -qnohot -qnostaticlink")
  else()
    message("Using release configuration for front end compilation")
    set(CMAKE_CXX_FLAGS_RELEASE "-O3 -q64 -qstrict -qnostaticlink -qnostaticlink=libgcc")
    set(CMAKE_C_FLAGS_RELEASE "-O3 -q64 -qstrict -qnostaticlink -qnostaticlink=libgcc")
  endif()
endif()

