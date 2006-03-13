
include $(TOP)/make/system.mk

MODULE = 

HEADER_SRC = \
	eq.h \
	base/barrier.h \
	base/base.h \
	base/clock.h \
	base/defines.h \
	base/gate.h \
	base/hash.h \
	base/idPool.h \
	base/lock.h \
	base/log.h \
	base/mtQueue.h \
	base/refPtr.h \
	base/referenced.h \
	base/requestHandler.h \
	base/sema.h \
	base/shm.h \
	base/thread.h \
	base/timedLock.h \
	net/barrier.h \
	net/base.h \
	net/commands.h \
	net/connection.h \
	net/connectionSet.h \
	net/connectionDescription.h \
	net/fdConnection.h \
	net/global.h \
	net/group.h \
	net/idHash.h \
	net/init.h \
	net/message.h \
	net/mobject.h \
	net/net.h \
	net/node.h \
	net/nodeID.h \
	net/object.h \
	net/packets.h \
	net/pipeConnection.h \
	net/requestCache.h \
	net/requestQueue.h \
	net/session.h \
	net/uniPipeConnection.h \
	net/versionedObject.h \
	client/channel.h \
	client/client.h \
	client/commands.h \
	client/config.h \
	client/configParams.h \
	client/frustum.h \
	client/global.h \
	client/init.h \
	client/node.h \
	client/nodeFactory.h \
	client/openParams.h \
	client/packets.h \
	client/pipe.h \
	client/pixelViewport.h \
	client/projection.h \
	client/renderContext.h \
	client/server.h \
	client/viewport.h \
	client/wall.h \
	client/window.h


SOURCES = \
	base/barrier.cpp \
	base/gate.cpp \
	base/idPool.cpp \
	base/lock.cpp \
	base/log.cpp \
	base/requestHandler.cpp \
	base/sema.cpp \
	base/thread.cpp \
	base/timedLock.cpp \
	net/barrier.cpp \
	net/base.cpp \
	net/connection.cpp \
	net/connectionDescription.cpp \
	net/connectionSet.cpp \
	net/fdConnection.cpp \
	net/global.cpp \
	net/init.cpp \
	net/launcher.cpp \
	net/node.cpp \
	net/object.cpp \
	net/pipeConnection.cpp \
	net/requestCache.cpp \
	net/requestQueue.cpp \
	net/session.cpp \
	net/socketConnection.cpp \
	net/uniPipeConnection.cpp \
	net/util.cpp \
	net/versionedObject.cpp \
	client/channel.cpp \
	client/client.cpp \
	client/config.cpp \
	client/configParams.cpp \
	client/global.cpp \
	client/init.cpp \
	client/node.cpp \
	client/openParams.cpp \
	client/pipe.cpp \
	client/server.cpp \
	client/wall.cpp \
	client/window.cpp


CXXFLAGS += $(WINDOW_SYSTEM_INCS)
LDFLAGS  += $(WINDOW_SYSTEM_LIBS)

TARGETS = $(HEADERS) $(DYNAMIC_LIB)

include $(TOP)/make/rules.mk
