##
# Copyright (c) 2010 Daniel Pfeifer, All rights reserved.
#
# This file is freely distributable without licensing fees and
# is provided without guarantee or warrantee expressed or implied.
# This file is -not- in the public domain.
##

include(PurpleInstallPDB)
include(PurpleExpandLibraries)

function(PURPLE_ADD_AMALGAMATION NAME)
  purple_expand_libraries(LIBRARIES ${ARGN})

  set(THIS_SOURCES)
  set(THIS_LIBRARIES)
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

    get_property(LINK_LIBRARIES TARGET ${LIBRARY} PROPERTY LINK_LIBRARIES)
	list(APPEND THIS_LIBRARIES ${LINK_LIBRARIES})

    get_property(DEFINITIONS TARGET ${LIBRARY} PROPERTY COMPILE_DEFINITIONS)
    list(APPEND THIS_DEFINITIONS ${DEFINITIONS})
  endforeach(LIBRARY)

  add_library(${NAME} SHARED ${THIS_SOURCES})
  set_target_properties(${NAME} PROPERTIES COMPILE_DEFINITIONS "${THIS_DEFINITIONS}")
  purple_expand_libraries(THIS_LINK_LIBRARIES ${THIS_LIBRARIES} EXCLUDE ${ARGN})
  target_link_libraries(${NAME} ${THIS_LINK_LIBRARIES})

  install(TARGETS ${NAME}
    ARCHIVE DESTINATION lib COMPONENT dev
    LIBRARY DESTINATION lib COMPONENT dev
    )
  install(TARGETS ${NAME}
      RUNTIME DESTINATION bin COMPONENT lib
      LIBRARY DESTINATION lib COMPONENT lib
      )
  purple_install_pdb(${NAME} DESTINATION bin COMPONENT dev)
endfunction(PURPLE_ADD_AMALGAMATION NAME)
