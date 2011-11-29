# Copyright (c) 2010 Daniel Pfeifer <daniel@pfeifer-mail.de>
#               2011 Stefan Eilemann <eile@eyescale.ch>

include(PurpleAddLibrary)

find_package(BISON REQUIRED)
find_package(FLEX  REQUIRED) 

bison_target(PARSER loader.y ${CMAKE_CURRENT_BINARY_DIR}/parser.cpp 
  COMPILE_FLAGS "-l -p eqLoader_")

flex_target(LEXER loader.l ${CMAKE_CURRENT_BINARY_DIR}/lexer.cpp)
add_flex_bison_dependency(LEXER PARSER)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_definitions(-DYY_NEVER_INTERACTIVE)

set(HEADERS
    api.h
    canvas.h
    channel.h
    compound.h
    config.h
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
    parser.cpp
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

set(SERVER_LIBRARIES shared Equalizer)
if(GPUSD_FOUND)
  list(APPEND SOURCES ${AUTOCONFIG})
  list(APPEND SERVER_LIBRARIES ${GPUSD_LIBRARIES})
  include_directories(${GPUSD_INCLUDE_DIRS})
  foreach(GPUSD_COMPONENT ${GPUSD_COMPONENTS})
    if(GPUSD_${GPUSD_COMPONENT}_FOUND)
      add_definitions(-DEQ_USE_GPUSD_${GPUSD_COMPONENT})
    endif()
  endforeach()
endif()

if(EQUALIZER_INSTALL_SERVER_HEADERS)
  purple_add_library(EqualizerServer SHARED
    HEADERS_PREFIX "eq/server/" HEADERS ${HEADERS} SOURCES ${SOURCES}
    LINK_LIBRARIES ${SERVER_LIBRARIES}
    )
else()
  purple_add_library(EqualizerServer SHARED
    HEADERS_PREFIX "eq/server/" SOURCES ${HEADERS} ${SOURCES}
    LINK_LIBRARIES ${SERVER_LIBRARIES}
    )
endif()

source_group(equalizers REGULAR_EXPRESSION equalizers.*)
source_group(\\ FILES CMakeLists.txt ${HEADERS} ${SOURCES})
