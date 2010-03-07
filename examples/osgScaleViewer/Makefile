
# Building these files is a huge memory hog - disable parallel build
.NOTPARALLEL: %.cpp

# Check for OpenSceneGraph installation
OSG = $(wildcard /usr/local/include/osg/ref_ptr /usr/include/osg/ref_ptr)
ifeq ($(findstring ref_ptr, $(OSG)),ref_ptr)

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

CXXFLAGS += -I../include $(WINDOW_SYSTEM_INCS) -I/usr/local/include
LDFLAGS  += -leq -lpthread $(WINDOW_SYSTEM_LIBS) -L/usr/local/lib \
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
