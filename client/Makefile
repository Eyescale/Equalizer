
include $(TOP)/make/system.mk

HEADER_SRC = \
	init.h \
	server.h

SOURCES = \
	init.cpp

LDFLAGS += -leqnet

all: $(HEADERS) $(STATIC_LIB) $(DYNAMIC_LIB) foo

foo:
	echo $(STATIC_LIB)

include $(TOP)/make/rules.mk
