# Copyright (c) 2011 Stefan Eilemann <eile@eyescale.ch>

IF(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/.git)
  FIND_PACKAGE(Git)
  IF(Git_FOUND)
    Git_WC_INFO(${CMAKE_CURRENT_SOURCE_DIR} EQ)
    SET(EQ_REVISION ${EQ_WC_REVISION_HASH})
    message(STATUS "  Revision ${EQ_REVISION}")
  ELSE()
    message(STATUS "No revision version support, git not found")
  ENDIF()
ENDIF()
