# Copyright (c) 2010 Daniel Pfeifer <daniel@pfeifer-mail.de>
#               2011-2013 Stefan Eilemann <eile@eyescale.ch>
#               2012 Daniel Nachbaur <danielnachbaur@gmail.com>

find_package(BISON REQUIRED)
find_package(FLEX  REQUIRED)

bison_target(PARSER loader.y ${CMAKE_CURRENT_BINARY_DIR}/parser.cpp
  COMPILE_FLAGS "-l -p eqLoader_")

flex_target(LEXER loader.l ${CMAKE_CURRENT_BINARY_DIR}/lexer.cpp)
add_flex_bison_dependency(LEXER PARSER)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_subdirectory(equalizers)

add_definitions(-DYY_NEVER_INTERACTIVE)

set(HEADERS
    api.h
    canvas.h
    channel.h
	channelListener.h
    compound.h
	compoundListener.h
	compoundVisitor.h
    config.h
	configVisitor.h
    connectionDescription.h
    frame.h
    frameData.h
    frustum.h
    frustumData.h
    global.h
    init.h
    layout.h
    loader.h
    log.h
    node.h
    observer.h
    pipe.h
    segment.h
    server.h
    state.h
    tileQueue.h
    types.h
    view.h
    visitorResult.h
    window.h
)

set(SOURCES
    ${BISON_PARSER_OUTPUTS}
    ${FLEX_LEXER_OUTPUTS}
    canvas.cpp
    changeLatencyVisitor.h
    channel.cpp
    channelListener.h
    channelStopFrameVisitor.h
    channelUpdateVisitor.cpp
    channelUpdateVisitor.h
    colorMask.h
    compound.cpp
    compoundActivateVisitor.h
    compoundExitVisitor.h
    compoundInitVisitor.cpp
    compoundInitVisitor.h
    compoundListener.h
    compoundUpdateDataVisitor.cpp
    compoundUpdateDataVisitor.h
    compoundUpdateInputVisitor.cpp
    compoundUpdateInputVisitor.h
    compoundUpdateOutputVisitor.cpp
    compoundUpdateOutputVisitor.h
    compoundVisitor.h
    config.cpp
    configBackupVisitor.h
    configDeregistrator.h
    configDestCompoundVisitor.h
    configRegistrator.h
    configRestoreVisitor.h
    configUnmapVisitor.h
    configUpdateDataVisitor.cpp
    configUpdateDataVisitor.h
    configUpdateSyncVisitor.h
    configUpdateVisitor.h
    configVisitor.h
    connectionDescription.cpp
    convert11Visitor.h
    convert12Visitor.h
    equalizers/dfrEqualizer.cpp
    equalizers/equalizer.cpp
    equalizers/framerateEqualizer.cpp
    equalizers/loadEqualizer.cpp
    equalizers/monitorEqualizer.cpp
    equalizers/treeEqualizer.cpp
    equalizers/viewEqualizer.cpp
    equalizers/tileEqualizer.cpp
    frame.cpp
    frameData.cpp
    frustum.cpp
    frustumData.cpp
    global.cpp
    init.cpp
    layout.cpp
    loader.cpp
    loader.l
    loader.y
    node.cpp
    nodeFactory.cpp
    nodeFactory.h
    nodeFailedVisitor.h
    observer.cpp
    pipe.cpp
    segment.cpp
    server.cpp
    startLocalServer.cpp
    tileQueue.cpp
    view.cpp
    window.cpp
)

set(AUTOCONFIG
    config/display.h
    config/display.cpp
    config/resources.h
    config/resources.cpp
    config/server.h
    config/server.cpp
)

set(SERVER_LIBRARIES Equalizer)
if(HWSD_FOUND)
  list(APPEND SOURCES ${AUTOCONFIG})
  list(APPEND SERVER_LIBRARIES ${HWSD_LIBRARIES})
  include_directories(${HWSD_INCLUDE_DIRS})
  add_definitions(-DEQ_USE_HWSD)
  foreach(HWSD_COMPONENT ${HWSD_COMPONENTS})
    add_definitions(-DEQ_USE_HWSD_${HWSD_COMPONENT})
  endforeach()
endif()

add_library(EqualizerServer SHARED ${HEADERS} ${SOURCES})
target_link_libraries(EqualizerServer ${SERVER_LIBRARIES})
set_target_properties(EqualizerServer
  PROPERTIES VERSION ${VERSION} SOVERSION ${VERSION_ABI})

if (MSVC)
  install(FILES ${CMAKE_BINARY_DIR}/bin/Debug/EqualizerServer.pdb DESTINATION bin COMPONENT lib CONFIGURATIONS Debug)
  install(FILES ${CMAKE_BINARY_DIR}/bin/RelWithDebInfo/EqualizerServer.pdb DESTINATION bin COMPONENT lib CONFIGURATIONS RelWithDebInfo)
endif()

install(TARGETS EqualizerServer
        ARCHIVE DESTINATION lib COMPONENT eqdev
        RUNTIME DESTINATION bin COMPONENT eqlib
        LIBRARY DESTINATION lib COMPONENT eqlib)
if(EQUALIZER_INSTALL_SERVER_HEADERS)
  install(FILES ${HEADERS} DESTINATION include/eq/server COMPONENT eqdev)
endif()

source_group(equalizers REGULAR_EXPRESSION equalizers.*)
source_group(\\ FILES CMakeLists.txt ${HEADERS} ${SOURCES})
