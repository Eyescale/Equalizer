##
# Copyright (c) 2010 Daniel Pfeifer <daniel@pfeifer-mail.de>
##

set(NET_ADD_SOURCES)
set(NET_ADD_LINKLIB)

if(Boost_FOUND)
  list(APPEND NET_ADD_SOURCES rspConnection.cpp)
  list(APPEND NET_ADD_LINKLIB ${Boost_SYSTEM_LIBRARY})
endif(Boost_FOUND)

if(WIN32)
  list(APPEND NET_ADD_SOURCES pgmConnection.cpp)
endif(WIN32)

purple_add_library(eq_net
  FORWARD HEADERS
    base.h
    barrier.h
    bufferConnection.h
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
    node.h
    nodeType.h
    object.h
    objectCM.h
    objectVersion.h
    packets.h
    pairConnection.h
    session.h
    types.h
    version.h
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
    masterCM.cpp
    mcipConnection.cpp
    namedPipeConnection.cpp
    node.cpp
    object.cpp
    objectCM.cpp
    objectDataIStream.cpp
    objectDeltaDataIStream.cpp
    objectDeltaDataOStream.cpp
    objectInstanceDataIStream.cpp
    objectInstanceDataOStream.cpp
    objectSlaveDataOStream.cpp
    objectVersion.cpp
    packets.cpp
    pairConnection.cpp
    pipeConnection.cpp
    session.cpp
    socketConnection.cpp
    staticMasterCM.cpp
    staticSlaveCM.cpp
    unbufferedMasterCM.cpp
    versionedSlaveCM.cpp
    pgmConnection.h
    ${NET_ADD_SOURCES}
  LINK_LIBRARIES
    ${NET_ADD_LINKLIB}
    shared eq_base
  HEADERS_PREFIX "eq/net/"
  INCLUDE_ALL_HEADER eq/net.h
  )
