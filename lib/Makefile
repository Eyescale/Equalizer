

include $(TOP)/make/system.mk

MODULE = eq

HEADER_SRC = \
	plugins/compressor.h \
	eq.h \
	base/atomic.h \
	base/base.h \
	base/buffer.h \
	base/compressor.h \
	base/clock.h \
	base/compareAndSwap.h \
	base/debug.h \
	base/defines.h \
	base/dso.h \
	base/executionListener.h \
	base/file.h \
	base/global.h \
	base/hash.h \
	base/idPool.h \
	base/init.h \
	base/launcher.h \
	base/lfQueue.h \
	base/lock.h \
	base/lockable.h \
	base/log.h \
	base/memoryMap.h \
	base/monitor.h \
	base/mtQueue.h \
	base/nonCopyable.h \
	base/omp.h \
	base/perThread.h \
	base/perThreadRef.h \
	base/pluginRegistry.h \
	base/refPtr.h \
	base/referenced.h \
	base/requestHandler.h \
	base/rng.h \
	base/scopedMutex.h \
	base/sleep.h \
	base/spinLock.h \
	base/stdExt.h \
	base/thread.h \
	base/timedLock.h \
	base/types.h \
	base/uuid.h \
	net/barrier.h \
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
	net/connectionListener.h \
	net/connectionSet.h \
	net/connectionType.h \
	net/dataStream.h \
	net/dataIStream.h \
	net/dataOStream.h \
	net/dispatcher.h \
	net/global.h \
	net/init.h \
	net/instanceCache.h \
	net/log.h \
	net/net.h \
	net/node.h \
	net/nodeType.h \
	net/object.h \
	net/objectCM.h\
	net/objectVersion.h \
	net/packets.h \
	net/pairConnection.h \
	net/rspConnection.h \
	net/session.h \
	net/types.h \
	net/udpConnection.h \
	net/version.h \
	client/canvas.h \
	client/canvasVisitor.h \
	client/channel.h \
	client/channelStatistics.h \
	client/channelVisitor.h \
	client/client.h \
	client/colorMask.h \
	client/commandQueue.h \
	client/commands.h \
	client/compositor.h \
	client/computeContext.h \
	client/config.h \
	client/configEvent.h \
	client/configParams.h \
	client/configStatistics.h \
	client/configVisitor.h \
	client/cudaContext.h \
	client/drawableConfig.h \
	client/event.h \
	client/eventHandler.h \
	client/eye.h \
	client/frame.h \
	client/frameData.h \
	client/frustum.h \
	client/global.h \
	client/glWindow.h \
	client/image.h \
	client/init.h \
	client/layout.h \
	client/layoutVisitor.h \
	client/log.h \
	client/messagePump.h \
	client/node.h \
	client/nodeFactory.h \
	client/nodeType.h \
	client/nodeVisitor.h \
	client/object.h \
	client/observer.h \
	client/observerVisitor.h \
	client/os.h \
	client/osPipe.h \
	client/osWindow.h \
	client/packets.h \
	client/pipe.h \
	client/pipeVisitor.h \
	client/pixel.h \
	client/pixelViewport.h \
	client/projection.h \
	client/range.h \
	client/renderContext.h \
	client/segment.h \
	client/segmentVisitor.h \
	client/server.h \
	client/statistic.h \
	client/statisticSampler.h \
	client/subPixel.h \
	client/task.h \
	client/types.h \
	client/version.h \
	client/view.h \
	client/viewport.h \
	client/viewVisitor.h \
	client/visitorResult.h \
	client/wall.h \
	client/window.h \
	client/windowStatistics.h \
	client/windowSystem.h \
	client/windowVisitor.h \
	client/zoom.h \
	util/accum.h \
	util/accumBufferObject.h \
	util/bitmapFont.h \
	util/frameBufferObject.h \
	util/objectManager.h \
	util/texture.h \
	util/types.h \
	util/util.h

CXXFILES = \
	base/compressor.cpp \
	base/debug.cpp \
	base/dso.cpp \
	base/global.cpp \
	base/idPool.cpp \
	base/init.cpp \
	base/launcher.cpp \
	base/lock.cpp \
	base/log.cpp \
	base/memoryMap.cpp \
	base/monitor.cpp \
	base/omp.cpp \
	base/pluginRegistry.cpp \
	base/referenced.cpp \
	base/requestHandler.cpp \
	base/rng.cpp \
	base/thread.cpp \
	base/timedLock.cpp \
	base/uuid.cpp \
	net/barrier.cpp \
	net/bufferConnection.cpp \
	net/command.cpp \
	net/commandCache.cpp \
	net/commandQueue.cpp \
	net/connection.cpp \
	net/connectionDescription.cpp \
	net/connectionSet.cpp \
	net/dataStream.cpp \
	net/dataIStream.cpp \
	net/dataOStream.cpp \
	net/deltaMasterCM.cpp \
	net/dispatcher.cpp \
	net/eventConnection.cpp \
	net/fdConnection.cpp \
	net/fullMasterCM.cpp \
	net/global.cpp \
	net/init.cpp \
	net/instanceCache.cpp \
	net/mcipConnection.cpp \
	net/namedPipeConnection.cpp \
	net/node.cpp \
	net/object.cpp \
	net/objectCM.cpp \
	net/objectDataIStream.cpp \
	net/objectDeltaDataIStream.cpp \
	net/objectDeltaDataOStream.cpp \
	net/objectInstanceDataIStream.cpp \
	net/objectInstanceDataOStream.cpp \
	net/objectVersion.cpp \
	net/packets.cpp \
	net/pairConnection.cpp \
	net/pipeConnection.cpp \
	net/rspConnection.cpp \
	net/session.cpp \
	net/socketConnection.cpp \
	net/staticMasterCM.cpp \
	net/staticSlaveCM.cpp \
	net/udpConnection.cpp \
	net/unbufferedMasterCM.cpp \
	net/versionedSlaveCM.cpp \
	client/canvas.cpp \
	client/channel.cpp \
	client/channelStatistics.cpp \
	client/client.cpp \
	client/colorMask.cpp \
	client/commandQueue.cpp \
	client/compositor.cpp \
	client/computeContext.cpp \
	client/config.cpp \
	client/configDeserializer.cpp \
	client/configEvent.cpp \
	client/configParams.cpp \
	client/configStatistics.cpp \
	client/cudaContext.cpp \
	client/event.cpp \
	client/eventHandler.cpp \
	client/eye.cpp \
	client/frame.cpp \
	client/frameData.cpp \
	client/frustum.cpp \
	client/global.cpp \
	client/glWindow.cpp \
	client/pixel.cpp \
	client/image.cpp \
	client/init.cpp \
	client/jitter.cpp \
	client/layout.cpp \
	client/node.cpp \
	client/nodeFactory.cpp \
	client/frameDataStatistics.cpp \
	client/object.cpp \
	client/observer.cpp \
	client/os.cpp \
	client/osPipe.cpp \
	client/osWindow.cpp \
	client/pipe.cpp \
	client/pipeStatistics.cpp \
	client/projection.cpp \
	client/range.cpp \
	client/roiFinder.cpp \
	client/roiTracker.cpp \
	client/roiEmptySpaceFinder.cpp \
	client/renderContext.cpp \
	client/segment.cpp \
	client/server.cpp \
	client/statistic.cpp \
	client/subPixel.cpp \
	client/version.cpp \
	client/view.cpp \
	client/viewport.cpp \
	client/wall.cpp \
	client/window.cpp \
	client/windowStatistics.cpp \
	client/windowSystem.cpp \
	client/zoom.cpp \
	util/accum.cpp \
	util/accumBufferObject.cpp \
	util/bitmapFont.cpp \
	util/frameBufferObject.cpp \
	util/objectManager.cpp \
	util/texture.cpp \
	compressor/compressor.cpp \
	compressor/compressorRLEB.cpp \
	compressor/compressorRLE4B.cpp \
	compressor/compressorRLE4BU.cpp \
	compressor/compressorRLE565.cpp \
	compressor/compressorRLE10A2.cpp \
	compressor/compressorRLE4HF.cpp

SHADERS = \
	client/roiFragmentShader_glsl.h \
	client/roiFragmentShaderRGB_glsl.h


CFILES += \
	glew/glew.c

ifeq ($(findstring GLX, $(WINDOW_SYSTEM)),GLX)
  HEADER_SRC += \
	client/glXEventHandler.h \
	client/glXMessagePump.h \
	client/glXWindow.h \
	client/glXWindowEvent.h \
	client/glXPipe.h

  CXXFILES += \
	client/glXEventHandler.cpp \
	client/glXMessagePump.cpp \
	client/glXWindow.cpp \
	client/glXPipe.cpp
endif

ifeq ($(findstring AGL, $(WINDOW_SYSTEM)),AGL)
  HEADER_SRC += \
	client/aglEventHandler.h \
	client/aglMessagePump.h \
	client/aglWindow.h \
	client/aglWindowEvent.h \
	client/aglPipe.h

  CXXFILES += \
	client/aglEventHandler.cpp \
	client/aglMessagePump.cpp \
	client/aglWindow.cpp \
	client/aglPipe.cpp
endif

ifeq ($(findstring WGL, $(WINDOW_SYSTEM)),WGL)
  HEADER_SRC += \
	client/wglEventHandler.h \
	client/wglMessagePump.h \
	client/wglWindow.h \
	client/wglWindowEvent.h \
	client/wglPipe.h

  CXXFILES += \
	client/wglEventHandler.cpp \
	client/wglMessagePump.cpp \
	client/wglWindow.cpp \
	client/wglPipe.cpp

ifdef VARIANT
$(OBJECTS): $(PTHREAD_LIBS)

$(BUILD_DIR)/$(VARIANT)/lib/pthread.%: $(TOP)/Windows/pthreads/lib/pthreadGC2.%
	@mkdir -p $(@D)
	@echo 'Library file $@'
	@cp $< $@
endif
endif


CXXFLAGS += $(WINDOW_SYSTEM_INCS) -DEQUALIZER_EXPORTS
CFLAGS   += -DGLEW_BUILD -DEQ_DEFINES_H
LDFLAGS  += $(WINDOW_SYSTEM_LIBS)

TARGETS = $(SHADERS) $(HEADERS) $(DYNAMIC_LIB) subdirs

include $(TOP)/make/rules.mk
