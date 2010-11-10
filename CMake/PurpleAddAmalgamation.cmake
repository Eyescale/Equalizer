##
# Copyright (c) 2010 Daniel Pfeifer, All rights reserved.
#
# This file is freely distributable without licensing fees and
# is provided without guarantee or warrantee expressed or implied.
# This file is -not- in the public domain.
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
    get_property(SOURCE_DIRECTORY TARGET ${LIBRARY} PROPERTY SOURCE_DIRECTORY)

    foreach(SOURCE ${SOURCES})
      if(IS_ABSOLUTE ${SOURCE})
        list(APPEND THIS_SOURCES ${SOURCE})
      else(IS_ABSOLUTE ${SOURCE})
        list(APPEND THIS_SOURCES "${SOURCE_DIRECTORY}/${SOURCE}")
      endif(IS_ABSOLUTE ${SOURCE})
    endforeach(SOURCE ${SOURCES})

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
