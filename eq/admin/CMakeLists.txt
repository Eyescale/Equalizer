
# Copyright (c) 2013 Stefan.Eilemann@epfl.ch

set(ADMIN_HEADERS
    admin.h
    api.h
    base.h
    canvas.h
    channel.h
    client.h
    config.h
    init.h
    layout.h
    node.h
    observer.h
    pipe.h
    segment.h
    server.h
    types.h
    view.h
    visitorResult.h
    window.h
)

set(ADMIN_SOURCES
    canvas.cpp
    channel.cpp
    client.cpp
    config.cpp
    init.cpp
    layout.cpp
    node.cpp
    nodeFactory.cpp
    observer.cpp
    pipe.cpp
    segment.cpp
    server.cpp
    view.cpp
    window.cpp
)

source_group(fabric REGULAR_EXPRESSION .*)
source_group(\\ FILES CMakeLists.txt ${ADMIN_HEADERS} ${ADMIN_SOURCES})

add_library(EqualizerAdmin SHARED ${ADMIN_HEADERS} ${ADMIN_SOURCES})
target_link_libraries(EqualizerAdmin EqualizerFabric EqualizerServer)
set_target_properties(EqualizerAdmin
  PROPERTIES VERSION ${VERSION} SOVERSION ${VERSION_ABI})

install(FILES ${ADMIN_HEADERS} DESTINATION include/eq/admin COMPONENT dev)

if (MSVC)
  install(FILES ${CMAKE_BINARY_DIR}/bin/Debug/EqualizerAdmin.pdb
    DESTINATION bin COMPONENT dev CONFIGURATIONS Debug)
  install(FILES ${CMAKE_BINARY_DIR}/bin/RelWithDebInfo/EqualizerAdmin.pdb
    DESTINATION bin COMPONENT dev CONFIGURATIONS RelWithDebInfo)
endif()

install(TARGETS EqualizerAdmin
        ARCHIVE DESTINATION lib COMPONENT dev
        RUNTIME DESTINATION bin COMPONENT lib
        LIBRARY DESTINATION lib COMPONENT lib)
