
include $(TOP)/make/system.mk

HEADER_SRC = \
	channel.h \
	commands.h \
	config.h \
	configParams.h \
	eq.h \
	global.h \
	init.h \
	node.h \
	nodeFactory.h \
	packets.h \
	pipe.h \
	server.h \
	window.h

SOURCES = \
	channel.cpp \
	client.cpp \
	config.cpp \
	configParams.cpp \
	global.cpp \
	init.cpp \
	node.cpp \
	pipe.cpp \
	server.cpp \
	window.cpp

LDFLAGS += -leqnet

all: $(HEADERS) $(STATIC_LIB) $(DYNAMIC_LIB)

$(STATIC_LIB): $(SAMPLE_LIB_DIR)/libeqnet.a
$(DYNAMIC_LIB): $(SAMPLE_LIB_DIR)/libeqnet.a

include $(TOP)/make/rules.mk
