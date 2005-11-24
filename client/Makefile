
include $(TOP)/make/system.mk

HEADER_SRC = \
	commands.h \
	config.h \
	configParams.h \
	eq.h \
	global.h \
	init.h \
	node.h \
	nodeFactory.h \
	packets.h \
	server.h

SOURCES = \
	client.cpp \
	config.cpp \
	configParams.cpp \
	global.cpp \
	init.cpp \
	node.cpp \
	server.cpp

LDFLAGS += -leqnet

all: $(HEADERS) $(STATIC_LIB) $(DYNAMIC_LIB)

$(STATIC_LIB): $(SAMPLE_LIB_DIR)/libeqnet.a
$(DYNAMIC_LIB): $(SAMPLE_LIB_DIR)/libeqnet.a

include $(TOP)/make/rules.mk
