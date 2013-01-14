# Copyright (c) 2012 Stefan.Eilemann@epfl.ch

# installs files while preserving their relative directory
# Usage: install_files(<prefix> FILES <files> [COMPONENT <name>])
include(CMakeParseArguments)

function(INSTALL_FILES PREFIX)
  set(ARG_NAMES COMPONENT)
  set(ARGS_NAMES FILES)
  cmake_parse_arguments(THIS "" "${ARG_NAMES}" "${ARGS_NAMES}" ${ARGN})

  foreach(FILE ${THIS_FILES})
    string(REGEX MATCH "(.*)[/\\]" DIR ${FILE})
    if(THIS_COMPONENT)
      install(FILES ${FILE} DESTINATION ${PREFIX}/${DIR}
        COMPONENT ${THIS_COMPONENT})
    else()
      install(FILES ${FILE} DESTINATION ${PREFIX}/${DIR})
    endif()
  endforeach()
endfunction()
