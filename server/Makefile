
include $(TOP)/make/system.mk

SOURCES = \
	channel.cpp \
	config.cpp \
	compound.cpp \
	global.cpp \
	loader.cpp \
	node.cpp \
	pipe.cpp \
	server.cpp \
	window.cpp

CXXFILES = \
	eqs.cpp

MODULE  = server

CXXFLAGS    += $(WINDOW_SYSTEM_INCS)
LDFLAGS     += -leq
SA_LDFLAGS  += -leq -leqserver

ifeq "${ARCH}" "Linux"
SA_CXXFLAGS += -lrt
endif

TARGETS = $(DYNAMIC_LIB) $(SIMPLE_PROGRAMS)

include $(TOP)/make/rules.mk

$(SIMPLE_PROGRAMS): $(DYNAMIC_LIB)
