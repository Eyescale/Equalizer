
include $(TOP)/make/system.mk

HEADER_SRC = \
	config.h \
	configParams.h \
	eq.h \
	init.h \
	node.h \
	server.h

SOURCES = \
	configParams.cpp \
	init.cpp \
	node.cpp \
	server.cpp

LDFLAGS += -leqnet

all: $(HEADERS) $(STATIC_LIB) $(DYNAMIC_LIB)

include $(TOP)/make/rules.mk
