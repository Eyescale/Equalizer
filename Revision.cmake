# Copyright (c) 2011-2013 Stefan Eilemann <eile@eyescale.ch>

# Sets the following variables if git is found:
# GIT_REVISION: The current HEAD sha hash
# GIT_STATE: A description of the working tree, e.g., 1.8.0-48-g6d23f80-dirty
# GIT_ORIGIN_URL: The origin of the working tree

if(GIT_REVISION)
  return()
endif()

set(GIT_REVISION 0)
set(GIT_STATE)
set(GIT_ORIGIN_URL)

if(CMAKE_VERSION VERSION_LESS 2.8)
  message(STATUS "No revision version support, git not found")
  return()
endif()

if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/.git)
  find_package(Git)
  if(GIT_FOUND)
    execute_process( COMMAND ${GIT_EXECUTABLE} rev-parse --short HEAD
      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
      OUTPUT_VARIABLE GIT_REVISION OUTPUT_STRIP_TRAILING_WHITESPACE)
    execute_process( COMMAND ${GIT_EXECUTABLE} describe --long --tags --dirty
      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
      OUTPUT_VARIABLE GIT_STATE OUTPUT_STRIP_TRAILING_WHITESPACE)
    execute_process( COMMAND ${GIT_EXECUTABLE} config --get remote.origin.url
      OUTPUT_VARIABLE GIT_ORIGIN_URL
      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
    MESSAGE(STATUS "git revision ${GIT_REVISION} state ${GIT_STATE}")
  else()
    message(STATUS "No revision version support, git not found")
  endif()
endif()
