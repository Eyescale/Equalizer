
include $(TOP)/make/system.mk

HEADER_SRC = \
	commands.h \
	config.h \
	configParams.h \
	eq.h \
	init.h \
	node.h \
	packets.h \
	server.h

SOURCES = \
	config.cpp \
	configParams.cpp \
	init.cpp \
	node.cpp \
	server.cpp

LDFLAGS += -leqnet

all: $(HEADERS) $(STATIC_LIB) $(DYNAMIC_LIB)

#$(STATIC_LIB): $(LIBRARY_DIR)/libeqnet.a
#gd$(DYNAMIC_LIB): $(LIBRARY_DIR)/libeqnet.a

include $(TOP)/make/rules.mk
