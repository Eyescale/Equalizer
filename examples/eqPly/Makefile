
include $(TOP)/make/system.mk

PROGRAM = eqPly

SOURCES = \
	channel.cpp \
	initData.cpp \
	main.cpp \
	node.cpp \
	pipe.cpp \
	plyfile.cpp \
	plyFileIO.cpp

CXXFLAGS    += -I.. $(WINDOW_SYSTEM_INCS)
SA_LDFLAGS  += -leq -lpthread $(WINDOW_SYSTEM_LIBS)

ifeq "${ARCH}" "Linux"
SA_LDFLAGS += -lrt
endif

TARGETS = $(PROGRAMS)

include $(TOP)/make/rules.mk
