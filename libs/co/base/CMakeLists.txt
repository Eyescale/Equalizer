##
# Copyright (c) 2010 Daniel Pfeifer <daniel@pfeifer-mail.de>
# Copyright (c) 2010 Cedric Stalder <cedric.stalder@gmail.ch>
##
include(PurpleAddLibrary)
include(${CMAKE_CURRENT_SOURCE_DIR}/files.cmake)

source_group( "plugin" FILES ${PLUGIN_HEADERS} )
source_group( "base" FILES ${COBASE_PUBLIC_HEADERS} ${COBASE_HEADERS} ${COBASE_SOURCES})

set(COBASE_LINK_LIBS)
if(LINUX)
  list(APPEND COBASE_LINK_LIBS dl rt)
endif()

set(COBASE_PRECOMPILE)
if(WIN32)
  list(APPEND COBASE_PRECOMPILE precompile.h)
else()
  list(APPEND COBASE_PRECOMPILE <pthread.h> precompile.h)
endif()

purple_add_library(co_base ${CO_LIB_VARIANT}
  HEADERS_PREFIX "co/base/"
  HEADERS ${COBASE_PUBLIC_HEADERS}
  PRECOMPILE_HEADERS ${COBASE_PRECOMPILE}
  SOURCES ${COBASE_SOURCES} ${COBASE_HEADERS}
  LINK_LIBRARIES ${PTHREAD_LIBRARIES} ${COBASE_LINK_LIBS}
  )
set_target_properties(lib_co_base_shared PROPERTIES FOLDER "Transitional")
