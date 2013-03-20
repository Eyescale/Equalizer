
# Copyright (c) 2013 Stefan.Eilemann@epfl.ch
# finds the ittnotify API
# if not found, installs a dummy ittnotify.h to build_dir/include to eliminate
# the need to protect the itt calls with ifdefs

include(UpdateFile)

if(CMAKE_VERSION VERSION_LESS 2.8.3) # WAR bug
  get_filename_component(CMAKE_CURRENT_LIST_DIR ${CMAKE_CURRENT_LIST_FILE} PATH)
endif()

if(VTune_FIND_REQUIRED)
  set(_vtune_output 1)
  set(VTUNE_FIND_REQUIRED 1)
elseif(NOT VTune_FIND_QUIETLY)
  set(_vtune_output 1)
endif()

find_program(VTUNE_EXECUTABLE amplxe-cl)
if(NOT VTUNE_EXECUTABLE)
  set(VTUNE_FOUND)
  update_file(${CMAKE_CURRENT_LIST_DIR}/ittnotify.h
    ${CMAKE_BINARY_DIR}/include/ittnotify.h)
  add_definitions(-DINTEL_NO_ITTNOTIFY_API)
  return()
endif()

get_filename_component(VTUNE_DIR ${VTUNE_EXECUTABLE} PATH)
set(VTUNE_DIR "${VTUNE_DIR}/..")

find_path(VTUNE_INCLUDE_DIR ittnotify.h
  HINTS ${VTUNE_DIR}/include
  PATHS /usr/include /usr/local/include /opt/local/include)

find_library(VTUNE_ITTNOTIFY_LIBRARY NAMES ittnotify
  HINTS ${VTUNE_DIR}/lib64
  PATHS /usr/lib /usr/local/lib /opt/local/lib)

include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(VTUNE DEFAULT_MSG
  VTUNE_INCLUDE_DIR VTUNE_ITTNOTIFY_LIBRARY)

set(VTUNE_INCLUDE_DIRS ${VTUNE_INCLUDE_DIR})
set(VTUNE_LIBRARIES ${VTUNE_ITTNOTIFY_LIBRARY})
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -DINTEL_NO_ITTNOTIFY_API")
set(CMAKE_CXX_FLAGS_RELEASE
  "${CMAKE_CXX_FLAGS_RELEASE} -DINTEL_NO_ITTNOTIFY_API")

if(_vtune_output AND VTUNE_FOUND)
  message(STATUS "Found VTune in ${VTUNE_INCLUDE_DIRS};${VTUNE_LIBRARIES}")
endif()
