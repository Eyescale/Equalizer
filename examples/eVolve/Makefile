
include $(TOP)/make/system.mk

PROGRAM = eVolve

SOURCES = \
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
	vertexShaderComplex_cg.h \
	vertexShaderSimple_cg.h \
	fragmentShader_cg.h \
	fragmentShader_glsl.h \
	vertexShader_glsl.h

CXXFLAGS    += -I../include $(WINDOW_SYSTEM_INCS)
SA_LDFLAGS  += -leq -lpthread $(WINDOW_SYSTEM_LIBS)


ifdef CG_INSTALLED

SOURCES      += ../common/gloo/cg_program.cpp
CXXFLAGS     += -DCG_INSTALLED

ifeq "${ARCH}" "Darwin"
  LDFLAGS += -framework Cg
endif


ifeq "${ARCH}" "Linux"
  SA_LDFLAGS += -lrt
  LDFLAGS    += -lCg -lCgGL
ifeq "${SUBARCH}" "ia64"
    SA_LDFLAGS += -luuid
endif
endif
endif

TARGETS = $(PROGRAMS)

include $(TOP)/make/rules.mk

$(OBJECTS): $(SHADERS)

README: ../../../website/build/documents/user/eVolve.html
	links -dump $< > $@

%_cg.h: %.cg
	./stringify.pl $< > $@
%_glsl.h: %.glsl
	./stringify.pl $< > $@
