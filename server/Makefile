
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

MODULE  = server

CXXFLAGS    += $(WINDOW_SYSTEM_INCS)
LDFLAGS     += -L$(LIBRARY_DIR) -leq
SA_LDFLAGS  += -L$(SAMPLE_LIB_DIR) -leq -leqserver

ifeq "${ARCH}" "Linux"
SA_CXXFLAGS += -lrt
endif

TARGETS = $(DYNAMIC_LIB) eqs

eqs: $(DYNAMIC_LIB)

include $(TOP)/make/rules.mk
