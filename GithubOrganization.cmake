# Copyright (c) 2012 Stefan Eilemann <eile@eyescale.ch>
# sets GIT_ORIGIN_ORG based on origin remote github.com/<Org>/...
# sets GIT_ORIGIN_org to lower-case of GIT_ORIGIN_ORG

if(NOT GIT_ORIGIN_ORG)
  find_package(Git)
  if(GIT_EXECUTABLE)
    execute_process(COMMAND ${GIT_EXECUTABLE} config --get remote.origin.url
      WORKING_DIRECTORY ${CMAKE_SOURCE_DIR} OUTPUT_VARIABLE GIT_ORIGIN_URL)
    string(REGEX REPLACE ".*github.com[\\/:](.*)\\/.*" "\\1" GIT_ORIGIN_ORG
      "${GIT_ORIGIN_URL}")
  endif()
endif()

if(NOT GIT_ORIGIN_ORG OR GIT_ORIGIN_ORG STREQUAL GIT_ORIGIN_URL)
  message(STATUS "Can't determine github organization for ${GIT_ORIGIN_URL}")
  set(GIT_ORIGIN_ORG)
else()
  string(TOLOWER ${GIT_ORIGIN_ORG} GIT_ORIGIN_org)
endif()
