##
# Copyright (c) 2010 Daniel Pfeifer <daniel@pfeifer-mail.de>
##
include(${CMAKE_CURRENT_SOURCE_DIR}/files.cmake)
purple_add_library(eq_fabric ${EQ_LIB_VARIANT}
  HEADERS_PREFIX "eq/fabric/"
  FORWARD HEADERS
    ${EQ_FABRIC_FORWARD_HEADERS}
  SOURCES
    ${EQ_FABRIC_SOURCES}
  LINK_LIBRARIES
    shared co
  )
