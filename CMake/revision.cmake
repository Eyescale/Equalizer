# Copyright (c) 2011 Stefan Eilemann <eile@eyescale.ch>

IF(CMAKE_VERSION VERSION_LESS 2.8)
  MESSAGE(STATUS "No revision version support, git not found")
  return()
ENDIF()

IF(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/.git)
  FIND_PACKAGE(Git)
  IF(GIT_FOUND)
    EXECUTE_PROCESS(COMMAND ${GIT_EXECUTABLE} rev-parse --short HEAD
      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
      OUTPUT_VARIABLE EQ_REVISION OUTPUT_STRIP_TRAILING_WHITESPACE)
    MESSAGE(STATUS "git revision ${EQ_REVISION}")
  ELSE()
    MESSAGE(STATUS "No revision version support, git not found")
  ENDIF()
ENDIF()
