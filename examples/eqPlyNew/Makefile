
include $(TOP)/make/system.mk

PROGRAM = eqPly

SOURCES = \
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
	vertexBufferNode.cpp \
	vertexBufferRoot.cpp \
	vertexBufferLeaf.cpp


CXXFLAGS    += -I../include $(WINDOW_SYSTEM_INCS)
SA_LDFLAGS  += -leq -lpthread $(WINDOW_SYSTEM_LIBS)

ifeq "${ARCH}" "Linux"
  SA_LDFLAGS += -lrt
ifeq "${SUBARCH}" "ia64"
    SA_LDFLAGS += -luuid
endif
endif

TARGETS = $(PROGRAMS)

include $(TOP)/make/rules.mk

README: ../../../website/build/documents/user/eqPly.html
	links -dump $< > $@
