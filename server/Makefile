
include $(TOP)/make/system.mk

PROGRAM    = eqs

SOURCES = \
	channel.cpp \
	config.cpp \
	compound.cpp \
	eqs.cpp \
	node.cpp \
	pipe.cpp \
	server.cpp \
	window.cpp

MODULE  = server

CXXFLAGS    += $(WINDOW_SYSTEM_INCS)
LDFLAGS     += -L$(LIBRARY_DIR) -leq

ifeq "${ARCH}" "Linux"
SA_CXXFLAGS += -lrt
endif

TARGETS = $(DYNAMIC_LIB) $(PROGRAMS)

include $(TOP)/make/rules.mk
