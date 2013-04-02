
# Copyright (c) 2012-2013 Fabien Delalondre <fabien.delalondre@epfl.ch>
# Extension to support setting mpiwrappers as compilers
# If not, relies on the findMPI.cmake script to set libs, includes and compile flags
# Note: Improvement/evaluation must be made by comparing this script with:
# 1) http://www.na-mic.org/svn/Slicer3-lib-mirrors/trunk/CMake/Modules/FindMPI.cmake
# 2) http://www.openflipper.org/svnrepo/CoMISo/trunk/CoMISo/cmake/FindMPI.cmake

# This module will set the following variables:
#   MPI_WRAPPER_FOUND          TRUE if we have found MPI wrappers
#   MPI_FOUND                  TRUE if we have found MPI
#   MPI_COMPILE_FLAGS          Compilation flags for MPI programs
#   MPI_INCLUDE_PATH           Include path(s) for MPI header
#   MPI_LINK_FLAGS             Linking flags for MPI programs
#   MPI_LIBRARY                First MPI library to link against (cached)
#   MPI_EXTRA_LIBRARY          Extra MPI libraries to link against (cached)
#   MPI_LIBRARIES              All libraries to link MPI programs against


# Start with the list of MPI wrappers. Acknowledgement to http://www.openflipper.org/svnrepo/CoMISo/trunk/CoMISo/cmake/FindMPI.cmake
# Generic MPI compilers
set(_MPI_C_COMPILER_NAMES                  "mpicc|mpcc|mpicc_r|mpcc_r")
set(_MPI_CXX_COMPILER_NAMES                "mpicxx|mpiCC|mpcxx|mpCC|mpic[++]|mpc[++]")
# GNU compiler names
set(_MPI_GNU_C_COMPILER_NAMES              "mpigcc|mpgcc|mpigcc_r|mpgcc_r")
set(_MPI_GNU_CXX_COMPILER_NAMES            "mpig[++]|mpg[++]|mpig[++]_r|mpg[++]_r")
# Intel MPI compiler names
set(_MPI_Intel_C_COMPILER_NAMES            "mpiicc")
set(_MPI_Intel_CXX_COMPILER_NAMES          "mpiicpc|mpiicxx|mpiic[++]|mpiiCC")
# PGI compiler names
set(_MPI_PGI_C_COMPILER_NAMES              "mpipgcc|mppgcc")
set(_MPI_PGI_CXX_COMPILER_NAMES            "mpipgCC|mppgCC")
# XLC MPI Compiler names
set(_MPI_XL_C_COMPILER_NAMES               "mpxlc|mpxlc_r|mpixlc|mpixlc_r")
set(_MPI_XL_CXX_COMPILER_NAMES             "mpixlcxx|mpixlC|mpixlc[++]|mpxlcxx|mpxlc[++]|mpixlc[++]|mpxlCC|
                                           mpixlcxx_r|mpixlC_r|mpixlc[++]_r|mpxlcxx_r|mpxlc[++]_r|mpixlc[++]_r|mpxlCC_r")

# Find CC and CXX MPI wrappers
string(REGEX MATCH "${_MPI_C_COMPILER_NAMES}|${_MPI_GNU_C_COMPILER_NAMES}|${_MPI_Intel_C_COMPILER_NAMES}|
                    ${_MPI_PGI_C_COMPILER_NAMES}|${_MPI_XL_C_COMPILER_NAMES}" MPICC_COMPILER "${CMAKE_C_COMPILER}")
string(REGEX MATCH "${_MPI_CXX_COMPILER_NAMES}|${_MPI_GNU_CXX_COMPILER_NAMES}|${_MPI_Intel_CXX_COMPILER_NAMES}|
                    ${_MPI_PGI_CXX_COMPILER_NAMES}|${_MPI_XL_CXX_COMPILER_NAMES}" MPICXX_COMPILER "${CMAKE_CXX_COMPILER}")

# If one did not set up mpi wrappers as default compilers,
# trying to link to MPI libs
if("${MPICC_COMPILER}" STREQUAL "" AND "${MPICXX_COMPILER}" STREQUAL "")
  message(STATUS "Could not find mpi wrappers in CMAKE_C_COMPILER or
CMAKE_CXX_COMPILER. Trying to load mpi libs by default")
  set(MPI_WRAPPER_FOUND FALSE CACHE BOOL "Did not find the MPI Wrappers")
  find_package(MPI REQUIRED)
  # Add mpi linking flags
  if(MPI_LINK_FLAGS)
    LIST(APPEND LINK_FLAGS ${MPI_LINK_FLAGS})
  endif()
  # Add mpi compilation flags
  if(MPI_COMPILE_FLAGS)
    LIST(APPEND CMAKE_C_FLAGS ${MPI_COMPILE_FLAGS})
    LIST(APPEND CMAKE_CXX_FLAGS ${MPI_COMPILE_FLAGS})
  endif()
  # Add MPI includes
  if(MPI_INCLUDE_PATH)
    INCLUDE_DIRECTORIES(${MPI_INCLUDE_PATH})
  endif()
else()
  # Setting wrappers and mpi found to true
  set(MPI_WRAPPER_FOUND TRUE CACHE BOOL "found the MPI Wrappers")
  set(MPI_FOUND TRUE CACHE BOOL "Found the MPI library")
endif()

if (MPI_INCLUDE_PATH AND MPI_LIBRARY)
  set(MPI_FOUND TRUE CACHE BOOL "Found the MPI library")
else (MPI_INCLUDE_PATH AND MPI_LIBRARY)
  set(MPI_FOUND FALSE CACHE BOOL "Did not find the MPI library")
endif (MPI_INCLUDE_PATH AND MPI_LIBRARY)
