
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

#$(STATIC_LIB): $(LIBRARY_DIR)/libeqnet.a
#gd$(DYNAMIC_LIB): $(LIBRARY_DIR)/libeqnet.a

include $(TOP)/make/rules.mk
