
# Copyright (c) 2012-2013 Fabien Delalondre <fabien.delalondre@epfl.ch>
# Extension to support setting mpiwrappers as compilers
# If not, relies on the findMPI.cmake script to set libs, includes and compile flags

string(REGEX MATCH "mpicc|mpixlc" MPICC_COMPILER "${CMAKE_C_COMPILER}")
string(REGEX MATCH "mpicxx|mpixlcxx" MPICXX_COMPILER "${CMAKE_CXX_COMPILER}")

# If one did not set up mpi wrappers as default compilers,
# trying to link to MPI libs
if("${MPICC_COMPILER}" STREQUAL "" AND "${MPICXX_COMPILER}" STREQUAL "")
  find_package(MPI REQUIRED)
  list(APPEND LINK_FLAGS "${MPI_LINK_FLAGS}")
  list(APPEND CMAKE_C_FLAGS "${MPI_COMPILE_FLAGS}")
  list(APPEND CMAKE_CXX_FLAGS "${MPI_COMPILE_FLAGS}")
  include_directories(${MPI_INCLUDE_PATH})
endif()
