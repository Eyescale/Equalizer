# Copyright (c) 2011 Stefan Eilemann <eile@eyescale.ch>

set(GIT_REVISION 0)

if(CMAKE_VERSION VERSION_LESS 2.8)
  message(STATUS "No revision version support, git not found")
  return()
endif()

if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/.git)
  find_package(Git)
  if(GIT_FOUND)
    execute_process(COMMAND ${GIT_EXECUTABLE} rev-parse --short HEAD
      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
      OUTPUT_VARIABLE GIT_REVISION OUTPUT_STRIP_TRAILING_WHITESPACE)
    MESSAGE(STATUS "git revision ${GIT_REVISION}")
  else()
    message(STATUS "No revision version support, git not found")
  endif()
endif()
