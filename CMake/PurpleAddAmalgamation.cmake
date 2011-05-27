##
# Copyright (c) 2010 Daniel Pfeifer, All rights reserved.
#               2011 Stefan Eilemann <eile@eyescale.ch>
#
# This file is freely distributable without licensing fees and
# is provided without guarantee or warrantee expressed or implied.
# This file is -not- in the public domain.
##

include(PurpleInstallPDB)
include(PurpleExpandLibraries)

function(PURPLE_ADD_AMALGAMATION NAME SHORT_NAME VERSION VERSION_ABI)
  purple_expand_libraries(LIBRARIES ${ARGN})

  set(THIS_SOURCES)
  set(THIS_LIBRARIES)
  if(MSVC OR XCODE_VERSION)
    set(THIS_DEFINITIONS EQ_DSO_NAME=\"${CMAKE_SHARED_LIBRARY_PREFIX}${NAME}${CMAKE_SHARED_LIBRARY_SUFFIX}\")
  else()
    if(APPLE)
      set(THIS_DEFINITIONS EQ_DSO_NAME=\"${CMAKE_SHARED_LIBRARY_PREFIX}${NAME}.${VERSION_ABI}${CMAKE_SHARED_LIBRARY_SUFFIX}\")
    else()
      set(THIS_DEFINITIONS EQ_DSO_NAME=\"${CMAKE_SHARED_LIBRARY_PREFIX}${NAME}${CMAKE_SHARED_LIBRARY_SUFFIX}.${VERSION_ABI}\")
    endif()
  endif()

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
  set_target_properties(${NAME} PROPERTIES
    COMPILE_DEFINITIONS "${THIS_DEFINITIONS}"
    VERSION ${VERSION} SOVERSION ${VERSION_ABI}
    FOLDER "Libraries"
    )

  purple_expand_libraries(THIS_LINK_LIBRARIES ${THIS_LIBRARIES} EXCLUDE ${ARGN})
  target_link_libraries(${NAME} ${THIS_LINK_LIBRARIES})

  install(TARGETS ${NAME}
    ARCHIVE DESTINATION lib COMPONENT ${SHORT_NAME}dev
    RUNTIME DESTINATION bin COMPONENT ${SHORT_NAME}lib
    LIBRARY DESTINATION lib COMPONENT ${SHORT_NAME}lib
    )
  purple_install_pdb(${NAME} DESTINATION bin COMPONENT ${SHORT_NAME}dev)
endfunction(PURPLE_ADD_AMALGAMATION NAME)
