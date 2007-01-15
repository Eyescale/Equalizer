
include $(TOP)/make/system.mk

PROGRAM = eqPly

SOURCES = \
	appInitData.cpp \
	channel.cpp \
	config.cpp \
	initData.cpp \
	main.cpp \
	node.cpp \
	pipe.cpp \
	plyfile.cpp \
	plyFileIO.cpp \
	tracker.cpp

CXXFLAGS    += -I.. $(WINDOW_SYSTEM_INCS)
SA_LDFLAGS  += -leq -lpthread $(WINDOW_SYSTEM_LIBS)

ifeq "${ARCH}" "Linux"
  SA_LDFLAGS += -lrt
ifeq "${SUBARCH}" "ia64"
    SA_LDFLAGS += -luuid
endif
endif

TARGETS = $(PROGRAMS)

include $(TOP)/make/rules.mk
