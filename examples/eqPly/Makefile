
include $(TOP)/make/system.mk

PROGRAM = eqPly

CXXFILES = \
	cameraAnimation.cpp \
	channel.cpp \
	config.cpp \
	configEvent.cpp \
	eqPly.cpp \
	frameData.cpp \
	initData.cpp \
	localInitData.cpp \
	main.cpp \
	node.cpp \
	pipe.cpp \
	plyfile.cpp \
	tracker.cpp \
	vertexBufferBase.cpp \
	vertexBufferDist.cpp \
	vertexBufferLeaf.cpp \
	vertexBufferNode.cpp \
	vertexBufferRoot.cpp \
	vertexData.cpp \
	view.cpp \
	window.cpp

SHADERS = \
	fragmentShader_glsl.h \
	vertexShader_glsl.h

PLYFILES = \
	$(SHARE_DIR)/data/rockerArm.ply \
	 $(SHARE_DIR)/data/screwdriver.ply

CXXFLAGS += -I../include $(WINDOW_SYSTEM_INCS)
LDFLAGS  += -leq -lpthread $(WINDOW_SYSTEM_LIBS)

ifeq "${ARCH}" "Linux"
  LDFLAGS += -lrt
endif

TARGETS = $(SHADERS) $(PROGRAMS) $(PLYFILES)

include $(TOP)/make/rules.mk

README: ../../../website/build/documents/user/eqPly.html
	links -dump $< > $@

$(SHARE_DIR)/data/%.ply: %.ply
	@mkdir -p $(@D)
	cp $< $@

