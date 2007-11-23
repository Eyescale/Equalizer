
include $(TOP)/make/system.mk

PROGRAM = eVolve

CXXFILES = \
	localInitData.cpp \
	channel.cpp \
	config.cpp \
	eVolve.cpp \
	initData.cpp \
	main.cpp \
	node.cpp \
	pipe.cpp \
	shader.cpp\
	rawVolModel.cpp \
	sliceClipping.cpp \
	window.cpp

SHADERS = \
	fragmentShader_glsl.h \
	vertexShader_glsl.h

MODELS = \
	$(SHARE_DIR)/data/Bucky32x32x32_d.raw \
	$(SHARE_DIR)/data/Bucky32x32x32_d.raw.vhf

CXXFLAGS += -I../include $(WINDOW_SYSTEM_INCS)
LDFLAGS  += -leq -lpthread $(WINDOW_SYSTEM_LIBS)


ifeq "${ARCH}" "Linux"
  LDFLAGS += -lrt
ifeq "${SUBARCH}" "ia64"
    LDFLAGS += -luuid
endif
endif

TARGETS = $(PROGRAMS) $(MODELS)

include $(TOP)/make/rules.mk

$(OBJECTS): $(SHADERS)

README: ../../../website/build/documents/user/eVolve.html
	links -dump $< > $@

%_glsl.h: %.glsl
	./stringify.pl $< > $@

$(SHARE_DIR)/data/Bucky32x32x32_d.%: Bucky32x32x32_d.%
	@mkdir -p $(@D)
	cp $< $@
