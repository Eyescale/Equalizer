
include $(TOP)/make/system.mk

PROGRAMS   = eqs
HEADER_SRC = \
	channel.h \
	config.h \
	node.h \
	pipe.h \
	server.h \
	window.h

SOURCES = \
	channel.cpp \
	config.cpp \
	compound.cpp \
	pipe.cpp \
	server.cpp \
	window.cpp

MODULE  = server

LDFLAGS     += -L$(LIBRARY_DIR) -leqnet 
SA_CXXFLAGS += -L$(SAMPLE_LIB_DIR) -leqserver

CLEAN = $(PROGRAMS)

all: $(PROGRAMS)

$(PROGRAMS): $(HEADERS) $(DYNAMIC_LIB) $(STATIC_LIB)
$(STATIC_LIB): $(SAMPLE_LIB_DIR)/libeqnet.a
$(DYNAMIC_LIB): $(SAMPLE_LIB_DIR)/libeqnet.a

include $(TOP)/make/rules.mk
