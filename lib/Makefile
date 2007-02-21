
include $(TOP)/make/system.mk

MODULE = 

HEADER_SRC = \
	eq.h \
	base/barrier.h \
	base/base.h \
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
	base/scopedMutex.h \
	base/sema.h \
	base/shm.h \
	base/stdExt.h \
	base/thread.h \
	base/timedLock.h \
	base/userdata.h \
	net/barrier.h \
	net/base.h \
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
	net/fdConnection.h \
	net/global.h \
	net/group.h \
	net/idHash.h \
	net/init.h \
	net/log.h \
	net/net.h \
	net/node.h \
	net/nodeID.h \
	net/object.h \
	net/objectCM.h\
	net/packets.h \
	net/pipeConnection.h \
	net/session.h \
	net/types.h \
	client/channel.h \
	client/client.h \
	client/colorMask.h \
	client/commands.h \
	client/config.h \
	client/configEvent.h \
	client/configParams.h \
	client/event.h \
	client/eventHandler.h \
	client/frame.h \
	client/frameData.h \
	client/global.h \
	client/image.h \
	client/init.h \
	client/log.h \
	client/matrix4.h \
	client/node.h \
	client/nodeFactory.h \
	client/packets.h \
	client/pipe.h \
	client/pixelViewport.h \
	client/projection.h \
	client/range.h \
	client/renderContext.h \
	client/server.h \
	client/statEvent.h \
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
	base/launcher.cpp \
	base/lock.cpp \
	base/log.cpp \
	base/requestHandler.cpp \
	base/sema.cpp \
	base/thread.cpp \
	base/timedLock.cpp \
	net/barrier.cpp \
	net/base.cpp \
	net/command.cpp \
	net/commandCache.cpp \
	net/commandQueue.cpp \
	net/connection.cpp \
	net/connectionDescription.cpp \
	net/connectionSet.cpp \
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
	net/packets.cpp \
	net/pipeConnection.cpp \
	net/session.cpp \
	net/socketConnection.cpp \
	net/staticMasterCM.cpp \
	net/staticSlaveCM.cpp \
	client/channel.cpp \
	client/client.cpp \
	client/colorMask.cpp \
	client/config.cpp \
	client/configEvent.cpp \
	client/configParams.cpp \
	client/eventHandler.cpp \
	client/frame.cpp \
	client/frameData.cpp \
	client/global.cpp \
	client/image.cpp \
	client/init.cpp \
	client/node.cpp \
	client/pipe.cpp \
	client/range.cpp \
	client/renderContext.cpp \
	client/server.cpp \
	client/statEvent.cpp \
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

