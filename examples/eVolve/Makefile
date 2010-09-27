
include $(TOP)/make/system.mk

PROGRAM = eVolve

CXXFILES = \
	channel.cpp \
	config.cpp \
	eVolve.cpp \
	frameData.cpp \
	framesOrderer.cpp \
	glslShaders.cpp \
	initData.cpp \
	localInitData.cpp \
	main.cpp \
	node.cpp \
	pipe.cpp \
	rawVolModel.cpp \
	rawVolModelRenderer.cpp \
	sliceClipping.cpp \
	window.cpp

SHADERS = \
	fragmentShader_glsl.h \
	vertexShader_glsl.h

MODELS = \
	$(SHARE_DIR)/data/Bucky32x32x32_d.raw \
	$(SHARE_DIR)/data/Bucky32x32x32_d.raw.vhf

CXXFLAGS += -I../include $(WINDOW_SYSTEM_INCS)
LDFLAGS  += -lEqualizer -lpthread $(WINDOW_SYSTEM_LIBS)

ifeq "${ARCH}" "Linux"
  LDFLAGS += -lrt
endif

TARGETS = $(SHADERS) $(PROGRAMS) $(MODELS)

include $(TOP)/make/rules.mk


README: ../../../website/build/documents/user/eVolve.html
	links -dump $< > $@

$(SHARE_DIR)/data/Bucky32x32x32_d.%: Bucky32x32x32_d.%
	@mkdir -p $(@D)
	cp $< $@
