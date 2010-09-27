
# Check for OpenSceneGraph installation
OSG = $(wildcard /usr/local/include/osg/GLBeginEndAdapter /usr/include/osg/GLBeginEndAdapter)
ifeq ($(findstring GLBeginEndAdapter, $(OSG)),GLBeginEndAdapter)

include $(TOP)/make/system.mk

PROGRAM = osgScaleViewer

CXXFILES = \
	osgScaleViewer.cpp \
	channel.cpp \
	config.cpp \
	frameData.cpp \
	initData.cpp \
	node.cpp \
	pipe.cpp \
	quad.cpp \
	sceneReader.cpp \
	sceneView.cpp \
	tracker.cpp \
	window.cpp \
	main.cpp

CXXFLAGS += -I../include $(WINDOW_SYSTEM_INCS) -isystem /usr/local/include
LDFLAGS  += -lEqualizer -lpthread $(WINDOW_SYSTEM_LIBS) -L/usr/local/lib \
            -losg -losgViewer -lOpenThreads -losgDB -losgUtil -losgGA

ifeq "${ARCH}" "Linux"
  LDFLAGS += -lrt
endif

TARGETS = $(PROGRAMS)

include $(TOP)/make/rules.mk

else # OSG

info:
	@echo "osgScaleViewer not compiled, missing OpenSceneGraph header files"

endif # !OSG
