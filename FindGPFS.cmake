
# Copyright (c) 2013 Stefan.Eilemann@epfl.ch
# finds the ittnotify API
# if not found, installs a dummy ittnotify.h to build_dir/include to eliminate
# the need to protect the itt calls with ifdefs

if(GPFS_FIND_REQUIRED)
  set(_gpfs_output 1)
elseif(NOT GPFS_FIND_QUIETLY)
  set(_gpfs_output 1)
endif()

find_path(GPFS_INCLUDE_DIR gpfs.h
  PATHS /usr/include /usr/local/include /opt/local/include)

find_library(GPFS_LIBRARY NAMES gpfs
  PATHS /usr/lib /usr/local/lib /opt/local/lib)

include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(GPFS DEFAULT_MSG GPFS_INCLUDE_DIR GPFS_LIBRARY)

set(GPFS_INCLUDE_DIRS ${GPFS_INCLUDE_DIR})
set(GPFS_LIBRARIES ${GPFS_LIBRARY})

if(_gpfs_output AND GPFS_FOUND)
  message(STATUS "Found GPFS in ${GPFS_INCLUDE_DIRS};${GPFS_LIBRARIES}")
endif()
