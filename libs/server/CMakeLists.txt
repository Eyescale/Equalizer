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

add_definitions(-DEQSERVER_EXPORTS -DYY_NEVER_INTERACTIVE)

purple_add_library(EqualizerServer SHARED
  HEADERS_PREFIX "eq/server/"
  SOURCES
    ${BISON_PARSER_OUTPUTS}
    ${FLEX_LEXER_OUTPUTS}
    api.h
    canvas.h
    changeLatencyVisitor.h
    channel.h
    channelListener.h
    channelStopFrameVisitor.h
    channelUpdateVisitor.h
    colorMask.h
    compound.h
    compoundActivateVisitor.h
    compoundExitVisitor.h
    compoundInitVisitor.h
    compoundListener.h
    compoundUpdateDataVisitor.h
    compoundUpdateInputVisitor.h
    compoundUpdateOutputVisitor.h
    compoundVisitor.h
    config.h
    configBackupVisitor.h
    configDeregistrator.h
    configDestCompoundVisitor.h
    configRegistrator.h
    configRestoreVisitor.h
    configUnmapVisitor.h
    configUpdateDataVisitor.h
    configUpdateSyncVisitor.h
    configUpdateVisitor.h
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
    loader.l
    loader.y
    log.h
    node.h
    nodeFactory.h
    nodeFailedVisitor.h
    observer.h
    pipe.h
    segment.h
    server.h
    serverVisitor.h
    state.h
    swapBarrier.h
    types.h
    view.h
    visitorResult.h
    window.h
    canvas.cpp
    channel.cpp
    channelUpdateVisitor.cpp
    compound.cpp
    compoundInitVisitor.cpp
    compoundUpdateDataVisitor.cpp
    compoundUpdateInputVisitor.cpp
    compoundUpdateOutputVisitor.cpp
    config.cpp
    configUpdateDataVisitor.cpp
    connectionDescription.cpp
    equalizers/dfrEqualizer.cpp
    equalizers/framerateEqualizer.cpp
    equalizers/equalizer.cpp
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
    node.cpp
    nodeFactory.cpp
    observer.cpp
    parser.cpp
    pipe.cpp
    segment.cpp
    server.cpp
    startLocalServer.cpp
    swapBarrier.cpp
    view.cpp
    window.cpp
  LINK_LIBRARIES
    Equalizer
  )
set_target_properties(lib_EqualizerServer_shared PROPERTIES
  FOLDER "Libraries" PROJECT_LABEL "Server")
source_group(\\ REGULAR_EXPRESSION .*)
