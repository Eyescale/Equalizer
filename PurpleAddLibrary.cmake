##
# Copyright (c) 2010 Daniel Pfeifer <daniel@pfeifer-mail.de>
##

include(ParseArguments)
include(PurpleForwardHeaders)

function(PURPLE_ADD_LIBRARY NAME)
  string(TOUPPER ${NAME} UPPER_NAME)

  parse_arguments(THIS
    "SOURCES;HEADERS;HEADERS_DESTINATION;LINK_LIBRARIES"
    "SHARED;STATIC;FRAMEWORK;FORWARD"
    ${ARGN}
    )

  #purple_doxygen(doc_${NAME}_external HTML ${HEADERS})
  #purple_doxygen(doc_${NAME}_internal HTML ${HEADERS} ${SOURCES})

  if(THIS_FORWARD)
    purple_forward_headers(${THIS_HEADERS_DESTINATION} ${THIS_HEADERS})
  endif(THIS_FORWARD)

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
      OUTPUT_NAME ${NAME} COMPILE_DEFINITIONS ${UPPER_NAME}_SHARED)
  endif(THIS_SHARED)

  if(THIS_STATIC)
    set(THIS_TARGET lib_${NAME}_static)
    list(APPEND THIS_TARGETS ${THIS_TARGET})

    add_library(${THIS_TARGET} STATIC ${THIS_HEADERS} ${THIS_SOURCES})
    set_target_properties(${THIS_TARGET} PROPERTIES
      OUTPUT_NAME ${NAME} COMPILE_DEFINITIONS ${UPPER_NAME}_STATIC PREFIX lib)
  endif(THIS_STATIC)

  foreach(HEADER ${THIS_HEADERS})
    string(REGEX MATCH "(.*)[/\\]" DIR ${HEADER})
    install(FILES ${HEADER}
      DESTINATION include/${THIS_HEADERS_DESTINATION}/${DIR} COMPONENT dev)
    #set_property(SOURCE ${HEADER} PROPERTY MACOSX_PACKAGE_LOCATION Headers/${DIR})
  endforeach(HEADER ${THIS_HEADERS})

  install(TARGETS ${THIS_TARGETS}
    ARCHIVE DESTINATION lib COMPONENT dev
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
    )
endfunction(PURPLE_ADD_LIBRARY NAME)
