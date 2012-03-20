# Copyright (c) 2011-2012 Stefan Eilemann <eile@eyescale.ch>

find_path(_ofed_INCLUDE_DIR rdma/rdma_verbs.h
  HINTS ${OFED_ROOT}/include
  PATHS /usr/include /usr/local/include /opt/local/include)
find_library(_rdma_LIBRARY NAMES rdmacm
  HINTS ${OFED_ROOT}/lib
  PATHS /usr/lib /usr/local/lib /opt/local/lib)
find_library(_ibverbs_LIBRARY NAMES ibverbs
  HINTS ${OFED_ROOT}/lib
  PATHS /usr/lib /usr/local/lib /opt/local/lib)

include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(OFED DEFAULT_MSG
  _ofed_INCLUDE_DIR _rdma_LIBRARY _ibverbs_LIBRARY)

set(OFED_INCLUDE_DIRS ${_ofed_INCLUDE_DIR})
set(OFED_LIBRARIES ${_rdma_LIBRARY})
list(APPEND OFED_LIBRARIES ${_ibverbs_LIBRARY})

if(OFED_FOUND)
  message(STATUS "Found OFED in ${OFED_INCLUDE_DIRS};${OFED_LIBRARIES}")
endif()
