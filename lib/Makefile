
include $(TOP)/make/system.mk

MODULE = 

HEADER_SRC = \
	eq.h \
	base/barrier.h \
	base/base.h \
	base/clock.h \
	base/defines.h \
	base/executionListener.h \
	base/hash.h \
	base/idPool.h \
	base/lock.h \
	base/log.h \
	base/monitor.h \
	base/mtQueue.h \
	base/nonCopyable.h \
	base/perThread.h \
	base/process.h \
	base/refPtr.h \
	base/referenced.h \
	base/requestHandler.h \
	base/scopedMutex.h \
	base/sema.h \
	base/shm.h \
	base/stdExt.h \
	base/thread.h \
	base/timedLock.h \
	base/userdata.h \
	net/barrier.h \
	net/base.h \
	net/commands.h \
	net/connection.h \
	net/connection.ipp \
	net/connectionSet.h \
	net/connectionDescription.h \
	net/fdConnection.h \
	net/global.h \
	net/group.h \
	net/idHash.h \
	net/init.h \
	net/log.h \
	net/message.h \
	net/net.h \
	net/node.h \
	net/nodeID.h \
	net/object.h \
	net/packets.h \
	net/pipeConnection.h \
	net/requestCache.h \
	net/requestQueue.h \
	net/session.h \
	net/types.h \
	net/uniPipeConnection.h \
	client/channel.h \
	client/client.h \
	client/commands.h \
	client/config.h \
	client/configEvent.h \
	client/configParams.h \
	client/event.h \
	client/eventThread.h \
	client/frame.h \
	client/frameBuffer.h \
	client/frustum.h \
	client/global.h \
	client/image.h \
	client/init.h \
	client/log.h \
	client/matrix4.h \
	client/node.h \
	client/nodeFactory.h \
	client/object.h \
	client/openParams.h \
	client/packets.h \
	client/pipe.h \
	client/pixelViewport.h \
	client/projection.h \
	client/range.h \
	client/renderContext.h \
	client/server.h \
	client/version.h \
	client/view.h \
	client/viewport.h \
	client/wall.h \
	client/window.h \
	client/windowEvent.h \
	client/windowSystem.h

SOURCES = \
	base/barrier.cpp \
	base/idPool.cpp \
	base/lock.cpp \
	base/log.cpp \
	base/process.cpp \
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
	net/nodeID.cpp \
	net/object.cpp \
	net/pipeConnection.cpp \
	net/requestCache.cpp \
	net/requestQueue.cpp \
	net/session.cpp \
	net/socketConnection.cpp \
	net/uniPipeConnection.cpp \
	net/util.cpp \
	client/channel.cpp \
	client/client.cpp \
	client/config.cpp \
	client/configEvent.cpp \
	client/configParams.cpp \
	client/eventThread.cpp \
	client/frame.cpp \
	client/frameBuffer.cpp \
	client/frustum.cpp \
	client/global.cpp \
	client/image.cpp \
	client/init.cpp \
	client/matrix4.cpp \
	client/node.cpp \
	client/nodeFactory.cpp \
	client/openParams.cpp \
	client/pipe.cpp \
	client/range.cpp \
	client/renderContext.cpp \
	client/server.cpp \
	client/version.cpp \
	client/view.cpp \
	client/wall.cpp \
	client/window.cpp

ifeq ($(findstring GLX, $(WINDOW_SYSTEM)),GLX)
  HEADER_SRC += \
	client/glXEventThread.h \
	client/X11Connection.h

  SOURCES += \
	client/glXEventThread.cpp
endif

CXXFLAGS += $(WINDOW_SYSTEM_INCS)
LDFLAGS  += $(WINDOW_SYSTEM_LIBS)

TARGETS = $(HEADERS) $(VMMLIB_HEADERS) $(DYNAMIC_LIB)

include $(TOP)/make/rules.mk

#----------------------------------------------------------------------
# External template math library

VMMLIB_HEADER_SRC = $(wildcard vmmlib/include/*.h)
VMMLIB_HEADERS    = \
	$(VMMLIB_HEADER_SRC:vmmlib/include/%=$(HEADER_DIR)/vmmlib/%) \
	$(HEADER_DIR)/vmmlib/LICENSE

$(HEADER_DIR)/vmmlib/%: vmmlib/include/%
	@mkdir -p $(@D)
	@echo 'VMMLib header file $@'
	@cp $< $@

$(HEADER_DIR)/vmmlib/LICENSE: vmmlib/LICENSE
	@mkdir -p $(@D)
	@echo 'VMMLib $@'
	@cp $< $@

#----------------------------------------------------------------------

$(DYNAMIC_LIB): $(HEADERS) $(VMMLIB_HEADERS)

