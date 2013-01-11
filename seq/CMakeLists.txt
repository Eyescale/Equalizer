# Copyright (c) 2011 Stefan Eilemann <eile@eyescale.ch>

set(DETAIL_FILES
    detail/application.cpp
    detail/channel.h
    detail/channel.cpp
    detail/config.h
    detail/config.cpp
    detail/masterConfig.h
    detail/masterConfig.cpp
    detail/node.h
    detail/node.cpp
    detail/objectMap.h
    detail/objectMap.cpp
    detail/pipe.h
    detail/pipe.cpp
    detail/renderer.h
    detail/renderer.cpp
    detail/slaveConfig.h
    detail/slaveConfig.cpp
    detail/view.h
    detail/view.cpp
    detail/window.h
    detail/window.cpp
  )

set(SEQUEL_HEADERS
    api.h
    application.h
    error.h
    objectType.h
    renderer.h
    sequel.h
    types.h
    viewData.h
  )
set(SEQUEL_SOURCES
    application.cpp
    error.cpp
    renderer.cpp
    viewData.cpp
  )

source_group(\\ REGULAR_EXPRESSION .*)
source_group(\\ FILES CMakeLists.txt ${SEQUEL_HEADERS} ${SEQUEL_SOURCES})
source_group(detail FILES ${DETAIL_FILES})

add_library(Sequel SHARED ${SEQUEL_HEADERS} ${SEQUEL_SOURCES} ${DETAIL_FILES})
target_link_libraries(Sequel Equalizer)
set_target_properties(Sequel
  PROPERTIES VERSION ${VERSION} SOVERSION ${VERSION_ABI})

install(FILES ${SEQUEL_HEADERS} DESTINATION include/seq COMPONENT dev)
install(TARGETS Sequel
        ARCHIVE DESTINATION lib COMPONENT dev
        RUNTIME DESTINATION bin COMPONENT lib
        LIBRARY DESTINATION lib COMPONENT lib)
