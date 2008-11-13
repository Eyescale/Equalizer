
include $(TOP)/make/system.mk

PROGRAM = eqPly

CXXFILES = \
	localInitData.cpp \
	channel.cpp \
	config.cpp \
	eqPly.cpp \
	initData.cpp \
	main.cpp \
	node.cpp \
	pipe.cpp \
	tracker.cpp \
	window.cpp \
	plyfile.cpp \
	vertexData.cpp \
	vertexBufferBase.cpp \
	vertexBufferDist.cpp \
	vertexBufferLeaf.cpp \
	vertexBufferNode.cpp \
	vertexBufferRoot.cpp

SHADERS = \
	fragmentShader_glsl.h \
	vertexShader_glsl.h


CXXFLAGS += -I../include $(WINDOW_SYSTEM_INCS)
LDFLAGS  += -leq -lpthread $(WINDOW_SYSTEM_LIBS)

ifeq "${ARCH}" "Linux"
  LDFLAGS += -lrt
ifeq "${SUBARCH}" "ia64"
    LDFLAGS += -luuid
endif
endif

TARGETS = $(SHADERS) $(PROGRAMS) $(SHARE_DIR)/data/rockerArm.ply

include $(TOP)/make/rules.mk

README: ../../../website/build/documents/user/eqPly.html
	links -dump $< > $@

$(SHARE_DIR)/data/rockerArm.ply: rockerArm.ply
	@mkdir -p $(@D)
	cp $< $@

%_glsl.h: %.glsl
	./stringify.pl $< > $@
