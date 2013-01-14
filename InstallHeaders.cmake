# Copyright (c) 2012 Stefan.Eilemann@epfl.ch

# installs headers while preserving their relative directory
# Usage: install_headers(<prefix> HEADERS <files> [COMPONENT <name>])
include(CMakeParseArguments)

function(INSTALL_HEADERS PREFIX)
  set(ARG_NAMES COMPONENT)
  set(ARGS_NAMES HEADERS)
  cmake_parse_arguments(THIS "" "${ARG_NAMES}" "${ARGS_NAMES}" ${ARGN})

  foreach(HEADER ${THIS_HEADERS})
    string(REGEX MATCH "(.*)[/\\]" DIR ${HEADER})
    if(THIS_COMPONENT)
      install(FILES ${HEADER} DESTINATION include/${PREFIX}/${DIR}
        COMPONENT ${THIS_COMPONENT})
    else()
      install(FILES ${HEADER} DESTINATION include/${PREFIX}/${DIR})
    endif()
  endforeach()
endfunction()
