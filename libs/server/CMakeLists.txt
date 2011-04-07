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
    config.h
    init.h
    canvas.h
    configBackupVisitor.h
    layout.h
    changeLatencyVisitor.h
    configDeregistrator.h
    loader.h
    loader.l
    loader.y
    channel.h
    configDestCompoundVisitor.h
    log.h
    channelListener.h
    configRegistrator.h
    node.h
    channelStopFrameVisitor.h
    configRestoreVisitor.h
    nodeFactory.h
    channelUpdateVisitor.h
    configUnmapVisitor.h
    observer.h
    colorMask.h
    configUpdateDataVisitor.h
    pipe.h
    compound.h
    configUpdateSyncVisitor.h
    segment.h
    compoundActivateVisitor.h
    configUpdateVisitor.h
    server.h
    compoundExitVisitor.h
    configVisitor.h
    serverVisitor.h
    compoundInitVisitor.h
    connectionDescription.h
    state.h
    compoundListener.h
    frame.h
    swapBarrier.h
    compoundUpdateDataVisitor.h
    frameData.h
    types.h
    compoundUpdateInputVisitor.h
    frustum.h
    view.h
    compoundUpdateOutputVisitor.h
    frustumData.h
    visitorResult.h
    compoundVisitor.h
    global.h
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
