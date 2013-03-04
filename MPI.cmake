
# Copyright (c) 2012-2013 Fabien Delalondre <fabien.delalondre@epfl.ch>
# Extension to support setting mpiwrappers as compilers
# If not, relies on the findMPI.cmake script to set libs, includes and compile flags

string(REGEX MATCH "mpicc|mpixlc" MPICC_COMPILER "${CMAKE_C_COMPILER}")
string(REGEX MATCH "mpicxx|mpixlcxx" MPICXX_COMPILER "${CMAKE_CXX_COMPILER}")

# Debugging
#message(STATUS "Value of MPICC_COMPILER: " ${MPICC_COMPILER})
#message(STATUS "Value of MPICXX_COMPILER: " ${MPICXX_COMPILER})

# If one did not set up mpi wrappers as default compilers, 
# trying to link to MPI libs
if("${MPICC_COMPILER}" STREQUAL "" AND "${MPICXX_COMPILER}" STREQUAL "")
  message(STATUS "Could not find mpi wrappers in CMAKE_C_COMPILER or
CMAKE_CXX_COMPILER. Trying to load mpi libs by default")
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
endif()


