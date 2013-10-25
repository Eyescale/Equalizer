# Copyright (c) 2012 Stefan Eilemann <eile@eyescale.ch>
# sets GIT_ORIGIN_ORG based on root or origin remote github.com/<Org>/...
# sets GIT_ORIGIN_org to lower-case of GIT_ORIGIN_ORG

include(Revision)

if(GIT_ROOT_URL)
  set(_git_origin_url ${GIT_ROOT_URL})
elseif(GIT_ORIGIN_URL)
  set(_git_origin_url ${GIT_ORIGIN_URL})
endif()

if(_git_origin_url)
  string(REGEX REPLACE ".*github.com[\\/:](.*)\\/.*" "\\1" GIT_ORIGIN_ORG
    "${_git_origin_url}")
endif()

if(NOT GIT_ORIGIN_ORG OR GIT_ORIGIN_ORG STREQUAL _git_origin_url)
  message(STATUS "Can't determine github organization for ${_git_origin_url}")
  set(GIT_ORIGIN_ORG)
else()
  string(TOLOWER ${GIT_ORIGIN_ORG} GIT_ORIGIN_org)
endif()
