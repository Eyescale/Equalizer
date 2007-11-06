
include $(TOP)/make/system.mk

MODULE = eq

HEADER_SRC = \
	eq.h \
	base/barrier.h \
	base/base.h \
	base/buffer.h \
	base/clock.h \
	base/debug.h \
	base/defines.h \
	base/executionListener.h \
	base/hash.h \
	base/idPool.h \
	base/launcher.h \
	base/lock.h \
	base/log.h \
	base/monitor.h \
	base/mtQueue.h \
	base/nonCopyable.h \
	base/perThread.h \
	base/refPtr.h \
	base/referenced.h \
	base/requestHandler.h \
	base/rng.h \
	base/scopedMutex.h \
	base/sema.h \
	base/spinLock.h \
	base/stdExt.h \
	base/thread.h \
	base/timedLock.h \
	base/types.h \
	base/userdata.h \
	net/barrier.h \
	net/base.h \
	net/bufferConnection.h \
	net/command.h \
	net/commandCache.h \
	net/commandFunc.h \
	net/commandQueue.h \
	net/commandResult.h \
	net/commands.h \
	net/connection.h \
	net/connection.ipp \
	net/connectionDescription.h \
	net/connectionSet.h \
	net/connectionType.h \
	net/dataIStream.h \
	net/dataOStream.h \
	net/fdConnection.h \
	net/global.h \
	net/idHash.h \
	net/init.h \
	net/log.h \
	net/net.h \
	net/node.h \
	net/nodeID.h \
	net/nodeType.h \
	net/object.h \
	net/objectCM.h\
	net/packets.h \
	net/pairConnection.h \
	net/pipeConnection.h \
	net/session.h \
	net/types.h \
	client/channel.h \
	client/client.h \
	client/colorMask.h \
	client/commandQueue.h \
	client/commands.h \
	client/config.h \
	client/configEvent.h \
	client/configParams.h \
	client/event.h \
	client/eventHandler.h \
	client/eye.h \
	client/frame.h \
	client/frameData.h \
	client/glFunctions.h \
	client/global.h \
	client/image.h \
	client/init.h \
	client/log.h \
	client/matrix4.h \
	client/messagePump.h \
	client/node.h \
	client/nodeFactory.h \
	client/nodeType.h \
	client/objectManager.h \
	client/packets.h \
	client/pipe.h \
	client/pixelViewport.h \
	client/range.h \
	client/renderContext.h \
	client/server.h \
	client/statEvent.h \
	client/version.h \
	client/viewport.h \
	client/window.h \
	client/windowEvent.h \
	client/windowSystem.h \
	client/X11Connection.h

SOURCES = \
	base/barrier.cpp \
	base/debug.cpp \
	base/idPool.cpp \
	base/launcher.cpp \
	base/lock.cpp \
	base/log.cpp \
	base/monitor.cpp \
	base/requestHandler.cpp \
	base/sema.cpp \
	base/spinLock.cpp \
	base/thread.cpp \
	base/timedLock.cpp \
	net/barrier.cpp \
	net/base.cpp \
	net/bufferConnection.cpp \
	net/command.cpp \
	net/commandCache.cpp \
	net/commandQueue.cpp \
	net/connection.cpp \
	net/connectionDescription.cpp \
	net/connectionSet.cpp \
	net/dataIStream.cpp \
	net/dataOStream.cpp \
	net/deltaMasterCM.cpp \
	net/deltaSlaveCM.cpp \
	net/fdConnection.cpp \
	net/fullMasterCM.cpp \
	net/fullSlaveCM.cpp \
	net/global.cpp \
	net/init.cpp \
	net/node.cpp \
	net/nodeID.cpp \
	net/object.cpp \
	net/objectCM.cpp \
	net/objectDataIStream.cpp \
	net/objectDeltaDataIStream.cpp \
	net/objectDeltaDataOStream.cpp \
	net/objectInstanceDataIStream.cpp \
	net/objectInstanceDataOStream.cpp \
	net/packets.cpp \
	net/pairConnection.cpp \
	net/pipeConnection.cpp \
	net/session.cpp \
	net/socketConnection.cpp \
	net/staticMasterCM.cpp \
	net/staticSlaveCM.cpp \
	client/channel.cpp \
	client/client.cpp \
	client/colorMask.cpp \
	client/commandQueue.cpp \
	client/config.cpp \
	client/configEvent.cpp \
	client/configParams.cpp \
	client/event.cpp \
	client/eventHandler.cpp \
	client/frame.cpp \
	client/frameData.cpp \
	client/glFunctions.cpp \
	client/global.cpp \
	client/image.cpp \
	client/init.cpp \
	client/messagePump.cpp \
	client/node.cpp \
	client/objectManager.cpp \
	client/pipe.cpp \
	client/range.cpp \
	client/renderContext.cpp \
	client/server.cpp \
	client/statEvent.cpp \
	client/version.cpp \
	client/window.cpp \
	client/windowEvent.cpp \
	client/windowSystem.cpp

ifeq ($(findstring GLX, $(WINDOW_SYSTEM)),GLX)
  HEADER_SRC += \
	client/glXEventThread.h

  SOURCES += \
	client/glXEventThread.cpp
endif

ifeq ($(findstring AGL, $(WINDOW_SYSTEM)),AGL)
  HEADER_SRC += \
	client/aglEventHandler.h

  SOURCES += \
	client/aglEventHandler.cpp
endif

ifeq ($(findstring WGL, $(WINDOW_SYSTEM)),WGL)
  HEADER_SRC += \
	client/glext.h \
	client/wglext.h \
	client/wglEventHandler.h

  SOURCES += \
	client/wglEventHandler.cpp

ifdef VARIANT
$(OBJECTS): $(PTHREAD_LIBS)

$(BUILD_DIR)/$(VARIANT)/lib/pthread.%: $(TOP)/Windows/pthreads/lib/pthreadGC2.%
	@mkdir -p $(@D)
	@echo 'Library file $@'
	@cp $< $@
endif
endif

CXXFLAGS += $(WINDOW_SYSTEM_INCS) -DEQUALIZER_EXPORTS
LDFLAGS  += $(WINDOW_SYSTEM_LIBS)

TARGETS = $(HEADERS) $(DYNAMIC_LIB)

include $(TOP)/make/rules.mk
