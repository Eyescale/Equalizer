# Copyright (c) 2010-2015 Daniel Pfeifer <daniel@pfeifer-mail.de>
#                         Stefan Eilemann <eile@eyescale.ch>
#                         Daniel Nachbaur <danielnachbaur@gmail.com>

common_find_package(BISON REQUIRED)
common_find_package(FLEX  REQUIRED)

bison_target(PARSER loader.y ${CMAKE_CURRENT_BINARY_DIR}/parser.cpp
  COMPILE_FLAGS "-l -p eqLoader_")

flex_target(LEXER loader.l ${CMAKE_CURRENT_BINARY_DIR}/lexer.cpp)
add_flex_bison_dependency(LEXER PARSER)
list(APPEND CPPCHECK_EXTRA_ARGS
  --suppress=*:${CMAKE_CURRENT_BINARY_DIR}/lexer.cpp
  --suppress=*:${CMAKE_CURRENT_BINARY_DIR}/parser.cpp
  --suppress=*:${CMAKE_CURRENT_SOURCE_DIR}/loader.l)

if(CPPCHECK_VERSION VERSION_LESS 1.68)
  list(APPEND CPPCHECK_EXTRA_ARGS --suppress=operatorEqVarError:
    ${CMAKE_CURRENT_SOURCE_DIR}/equalizers/equalizer.cpp)
endif()

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
add_definitions(-DYY_NEVER_INTERACTIVE)

set(PUBLIC_HEADERS
    canvas.h
    channel.h
    channelListener.h
    compound.h
    compoundListener.h
    compoundVisitor.h
    config.h
    configVisitor.h
    connectionDescription.h
    equalizers/equalizer.h
    equalizers/loadEqualizer.h
    equalizers/tileEqualizer.h
    equalizers/viewEqualizer.h
    frame.h
    frameData.h
    frustum.h
    frustumData.h
    global.h
    init.h
    layout.h
    loader.h
    localServer.h
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

set(EQUALIZERSERVER_HEADERS
    changeLatencyVisitor.h
    channel.cpp
    channelListener.h
    channelStopFrameVisitor.h
    channelUpdateVisitor.h
    colorMask.h
    compoundActivateVisitor.h
    compoundExitVisitor.h
    compoundInitVisitor.h
    compoundListener.h
    compoundUpdateDataVisitor.h
    compoundUpdateInputVisitor.h
    compoundUpdateOutputVisitor.h
    compoundVisitor.h
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
    convert11Visitor.h
    convert12Visitor.h
    nodeFactory.h
    nodeFailedVisitor.h
)

set(EQUALIZERSERVER_SOURCES
    ${BISON_PARSER_OUTPUTS}
    ${FLEX_LEXER_OUTPUTS}
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
    localServer.cpp
    node.cpp
    nodeFactory.cpp
    observer.cpp
    pipe.cpp
    segment.cpp
    server.cpp
    tileQueue.cpp
    view.cpp
    window.cpp
)

if(EQUALIZER_INSTALL_SERVER_HEADERS)
  set(EQUALIZERSERVER_PUBLIC_HEADERS ${PUBLIC_HEADERS})
else()
  list(APPEND EQUALIZERSERVER_HEADERS ${PUBLIC_HEADERS})
endif()

set(EQUALIZERSERVER_LINK_LIBRARIES PUBLIC EqualizerFabric)
if(HWSD_FOUND)
  list(APPEND EQUALIZERSERVER_HEADERS
    config/display.h config/resources.h config/server.h)
  list(APPEND EQUALIZERSERVER_SOURCES
    config/display.cpp config/resources.cpp config/server.cpp)

  set(_hwsd_components hwsd_net_sys)
  if(SERVUS_USE_DNSSD OR SERVUS_USE_AVAHI_CLIENT)
    list(APPEND _hwsd_components hwsd_gpu_dns_sd hwsd_net_dns_sd)
  endif()
  if(EQ_GLX_USED)
    list(APPEND _hwsd_components hwsd_gpu_glx)
  elseif(EQ_AGL_USED)
    list(APPEND _hwsd_components hwsd_gpu_cgl)
  elseif(EQ_WGL_USED)
    list(APPEND _hwsd_components hwsd_gpu_wgl)
  endif()

  list(APPEND EQUALIZERSERVER_LINK_LIBRARIES PRIVATE ${_hwsd_components})
  foreach(_component ${_hwsd_components})
    add_definitions(-DEQUALIZER_USE_${_component})
  endforeach()
endif()

set(EQUALIZERSERVER_INCLUDE_NAME eq/server)
set(EQUALIZERSERVER_NAMESPACE eqserver)
set(EQUALIZERSERVER_OMIT_LIBRARY_HEADER ON)

common_library(EqualizerServer)
target_compile_definitions(EqualizerServer PRIVATE EQUALIZERFABRIC_SHARED_INL)
if(CMAKE_COMPILER_IS_CLANG)
  target_compile_options(EqualizerServer PUBLIC -Wno-overloaded-virtual)
endif()
