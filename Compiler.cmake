# Copyright (c) 2012-2013 Fabien Delalondre <fabien.delalondre@epfl.ch>
# Sets compiler optimization, definition and warnings according to
# chosen compiler
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

set(COMMON_GCC_FLAGS
  "-Wall -Wextra -Winvalid-pch -Winit-self -Wno-unknown-pragmas")

# GCC (+clang)
if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_COMPILER_IS_CLANG)
  include(${CMAKE_CURRENT_LIST_DIR}/CompilerVersion.cmake)
  compiler_dumpversion(GCC_COMPILER_VERSION)
  if(GCC_COMPILER_VERSION VERSION_LESS 4.1)
    message(ERROR "GCC 4.1 or later required, found ${GCC_COMPILER_VERSION}")
  endif()
  if(NOT WIN32 AND NOT XCODE_VERSION AND NOT RELEASE_VERSION)
    set(COMMON_GCC_FLAGS "${COMMON_GCC_FLAGS} -Werror")
  endif()
  if(GCC_COMPILER_VERSION VERSION_GREATER 4.1)
    set(COMMON_GCC_FLAGS "${COMMON_GCC_FLAGS} -Wshadow")
  endif()
  if(CMAKE_COMPILER_IS_CLANG)
    set(COMMON_GCC_FLAGS
      "${COMMON_GCC_FLAGS} -Qunused-arguments -ferror-limit=5")
  endif()

  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${COMMON_GCC_FLAGS}")
  set(CMAKE_CXX_FLAGS
    "${CMAKE_CXX_FLAGS} ${COMMON_GCC_FLAGS} -Wnon-virtual-dtor -Wsign-promo")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-strict-aliasing")
  set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Wuninitialized")

  if(NOT GCC_COMPILER_VERSION VERSION_LESS 4.3)
    if(GCC_COMPILER_VERSION VERSION_LESS 4.7)
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
    else()
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
    endif()
  endif()
  if(CMAKE_COMPILER_IS_CLANG)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
  endif()

# icc
elseif(CMAKE_COMPILER_IS_INTEL)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${COMMON_GCC_FLAGS}")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${COMMON_GCC_FLAGS} -std=c++11 -Wno-deprecated -Wno-unknown-pragmas -Wshadow -fno-strict-aliasing -Wuninitialized -Wsign-promo -Wnon-virtual-dtor")

  # Release: automatically generate instructions for the highest
  # supported compilation host
  set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -xhost")
  set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -xhost")

# xlc/BlueGene/PPC
elseif(CMAKE_COMPILER_IS_XLCXX)
  # default: Maintain code semantics Fix to link dynamically. On the
  # next pass should add an if statement: 'if shared ...'.  Overriding
  # default release flags since the default were '-O -NDEBUG'. By
  # default, set flags for backend since this is the most common use
  # case
  option(XLC_BACKEND "Compile for BlueGene compute nodes using XLC compilers"
    ON)
  if(XLC_BACKEND)
    set(CMAKE_CXX_FLAGS_RELEASE
      "-O3 -qtune=qp -qarch=qp -q64 -qstrict -qnohot -qnostaticlink -DNDEBUG")
    set(CMAKE_C_FLAGS_RELEASE ${CMAKE_CXX_FLAGS_RELEASE})
  else()
    set(CMAKE_CXX_FLAGS_RELEASE
      "-O3 -q64 -qstrict -qnostaticlink -qnostaticlink=libgcc -DNDEBUG")
    set(CMAKE_C_FLAGS_RELEASE ${CMAKE_CXX_FLAGS_RELEASE})
  endif()
endif()

# Visual Studio
if(MSVC)
  add_definitions(
    /D_CRT_SECURE_NO_WARNINGS
    /D_SCL_SECURE_NO_WARNINGS
    /wd4068 # disable unknown pragma warnings
    /wd4244 # conversion from X to Y, possible loss of data
    /wd4800 # forcing value to bool 'true' or 'false' (performance warning)
    )

  # By default, do not warn when built on machines using only VS Express
  # http://cmake.org/gitweb?p=cmake.git;a=commit;h=fa4a3b04d0904a2e93242c0c3dd02a357d337f77
  if(NOT DEFINED CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS_NO_WARNINGS)
    set(CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS_NO_WARNINGS ON)
  endif()

  # By default, do not warn when built on machines using only VS Express
  # http://cmake.org/gitweb?p=cmake.git;a=commit;h=fa4a3b04d0904a2e93242c0c3dd02a357d337f77
  if(NOT DEFINED CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS_NO_WARNINGS)
    set(CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS_NO_WARNINGS ON)
  endif()

  # http://www.ogre3d.org/forums/viewtopic.php?f=2&t=60015&start=0
  if(RELEASE_VERSION)
    set(CMAKE_CXX_FLAGS "/DWIN32 /D_WINDOWS /W3 /Zm500 /EHsc /GR")
  else()
    set(CMAKE_CXX_FLAGS "/DWIN32 /D_WINDOWS /W3 /Zm500 /EHsc /GR /WX")
  endif()
endif()
