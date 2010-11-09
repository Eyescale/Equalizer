##
# Copyright (c) 2010 Daniel Pfeifer <daniel@pfeifer-mail.de>
##

include(ParseArguments)
include(PurpleExpandLibraries)

function(PURPLE_ADD_AMALGAMATION NAME)
  parse_arguments(THIS "LINK_LIBRARIES" "" ${ARGN})
  purple_expand_libraries(LIBRARIES ${THIS_DEFAULT_ARGS})

  set(THIS_SOURCES)
  set(THIS_DEFINITIONS)

  foreach(LIBRARY ${LIBRARIES})
    get_property(SOURCES TARGET ${LIBRARY} PROPERTY SOURCES)
    list(APPEND THIS_SOURCES ${SOURCES})
    get_property(DEFINITIONS TARGET ${LIBRARY} PROPERTY COMPILE_DEFINITIONS)
    list(APPEND THIS_DEFINITIONS ${DEFINITIONS})
  endforeach(LIBRARY ${ARGN})

  add_library(${NAME} SHARED ${THIS_SOURCES})
  set_target_properties(${NAME} PROPERTIES COMPILE_DEFINITIONS "${THIS_DEFINITIONS}")
  purple_expand_libraries(THIS_LIBRARIES ${THIS_LINK_LIBRARIES})
  target_link_libraries(${NAME} ${THIS_LIBRARIES})

  install(TARGETS ${THIS_TARGET}
    ARCHIVE DESTINATION lib COMPONENT dev
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
    )
endfunction(PURPLE_ADD_AMALGAMATION NAME)
