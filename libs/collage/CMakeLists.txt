##
# Copyright (c) 2010 Daniel Pfeifer <daniel@pfeifer-mail.de>
##

set(NET_ADD_SOURCES)
set(NET_ADD_LINKLIB)

if(Boost_FOUND)
  list(APPEND NET_ADD_SOURCES rspConnection.cpp)
  list(APPEND NET_ADD_LINKLIB ${Boost_SYSTEM_LIBRARY})
endif(Boost_FOUND)

if(MSVC)
  list(APPEND NET_ADD_SOURCES pgmConnection.cpp)
  list(APPEND NET_ADD_LINKLIB Ws2_32) 
endif(MSVC)

if(WIN32)
  list(APPEND NET_ADD_LINKLIB mswsock) 
endif(WIN32)

add_subdirectory(base)

set(PLUGIN_HEADERS
  plugins/compressor.h
  plugins/compressorTokens.h
  plugins/compressorTypes.h
  )
  
purple_add_library(eq_net ${EQ_LIB_VARIANT}
  HEADERS_PREFIX "co/"
  FORWARD HEADERS
    ${PLUGIN_HEADERS}
    api.h
    barrier.h
    bufferConnection.h
    co.h
    command.h
    commandCache.h
    commandFunc.h
    commandQueue.h
    commands.h
    connection.h
    connection.ipp
    connectionDescription.h
    connectionListener.h
    connectionSet.h
    connectionType.h
    dataIStream.h
    dataOStream.h
    dispatcher.h
    global.h
    init.h
    instanceCache.h
    log.h
    localNode.h
    node.h
    nodeType.h
    object.h
    objectCM.h
    objectVersion.h
    packets.h
    pairConnection.h
    types.h
    version.h
  PRECOMPILE_HEADERS
    precompile.h
  SOURCES
    barrier.cpp
    bufferConnection.cpp
    command.cpp
    commandCache.cpp
    commandQueue.cpp
    connection.cpp
    connectionDescription.cpp
    connectionSet.cpp
    dataIStream.cpp
    dataOStream.cpp
    deltaMasterCM.cpp
    dispatcher.cpp
    eventConnection.cpp
    fdConnection.cpp
    fullMasterCM.cpp
    global.cpp
    init.cpp
    instanceCache.cpp
    localNode.cpp
    masterCM.cpp
    mcipConnection.cpp
    namedPipeConnection.cpp
    node.cpp
    object.cpp
    objectCM.cpp
    objectDataIStream.cpp
    objectDataOStream.cpp
    objectDeltaDataOStream.cpp
    objectInstanceDataOStream.cpp
    objectSlaveDataOStream.cpp
    objectStore.cpp
    objectVersion.cpp
    packets.cpp
    pairConnection.cpp
    pipeConnection.cpp
    socketConnection.cpp
    staticMasterCM.cpp
    staticSlaveCM.cpp
    unbufferedMasterCM.cpp
    version.cpp
    versionedSlaveCM.cpp
    pgmConnection.h
    ${NET_ADD_SOURCES}
  LINK_LIBRARIES
    ${NET_ADD_LINKLIB}
    shared eq_base
  )
