##
# Copyright (c) 2010 Daniel Pfeifer, All rights reserved.
#
# This file is freely distributable without licensing fees and
# is provided without guarantee or warrantee expressed or implied.
# This file is -not- in the public domain.
##

include(ParseArguments)
include(PurpleForwardHeaders)

function(PURPLE_ADD_LIBRARY NAME)
  string(TOUPPER ${NAME} UPPER_NAME)

  parse_arguments(THIS
    "SOURCES;HEADERS;HEADERS_PREFIX;LINK_LIBRARIES;INCLUDE_ALL_HEADER"
    "SHARED;STATIC;FRAMEWORK;FORWARD"
    ${ARGN}
    )

  #purple_doxygen(doc_${NAME}_external HTML ${HEADERS})
  #purple_doxygen(doc_${NAME}_internal HTML ${HEADERS} ${SOURCES})

  if(THIS_FORWARD)
    purple_forward_headers(${THIS_HEADERS_PREFIX} ${THIS_HEADERS})
  endif(THIS_FORWARD)

  if(THIS_INCLUDE_ALL_HEADER)
    purple_include_all_header(${THIS_INCLUDE_ALL_HEADER}
      ${THIS_HEADERS_PREFIX} ${THIS_HEADERS})
  endif(THIS_INCLUDE_ALL_HEADER)

  if(NOT THIS_SHARED AND NOT THIS_STATIC)
    set(THIS_SHARED ON)
    set(THIS_STATIC ON)
  endif(NOT THIS_SHARED AND NOT THIS_STATIC)

  set(THIS_TARGETS)

  if(THIS_SHARED)
    set(THIS_TARGET lib_${NAME}_shared)
    list(APPEND THIS_TARGETS ${THIS_TARGET})

    add_library(${THIS_TARGET} SHARED ${THIS_HEADERS} ${THIS_SOURCES})
    purple_expand_libraries(LINK_LIBRARIES ${THIS_LINK_LIBRARIES})
    target_link_libraries(${THIS_TARGET} ${LINK_LIBRARIES})
    set_target_properties(${THIS_TARGET} PROPERTIES
      OUTPUT_NAME ${NAME}
      COMPILE_DEFINITIONS ${UPPER_NAME}_SHARED
      SOURCE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
      )
  endif(THIS_SHARED)

  if(THIS_STATIC)
    set(THIS_TARGET lib_${NAME}_static)
    list(APPEND THIS_TARGETS ${THIS_TARGET})

    add_library(${THIS_TARGET} STATIC ${THIS_HEADERS} ${THIS_SOURCES})
    set_target_properties(${THIS_TARGET} PROPERTIES
      OUTPUT_NAME ${NAME}
      COMPILE_DEFINITIONS ${UPPER_NAME}_STATIC
      SOURCE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
      PREFIX lib
      )
  endif(THIS_STATIC)

  foreach(HEADER ${THIS_HEADERS})
    string(REGEX MATCH "(.*)[/\\]" DIR ${HEADER})
    install(FILES ${HEADER}
      DESTINATION include/${THIS_HEADERS_PREFIX}/${DIR} COMPONENT dev)
    #set_property(SOURCE ${HEADER} PROPERTY MACOSX_PACKAGE_LOCATION Headers/${DIR})
  endforeach(HEADER ${THIS_HEADERS})

  install(TARGETS ${THIS_TARGETS}
    ARCHIVE DESTINATION lib COMPONENT dev
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
    )
endfunction(PURPLE_ADD_LIBRARY NAME)
