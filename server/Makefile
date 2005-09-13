
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
	server.cpp \
	window.cpp

MODULE  = server

LDFLAGS     += -L$(LIBRARY_DIR) -leqnet 
SA_CXXFLAGS += -L$(SAMPLE_LIB_DIR) -leqserver

CLEAN = $(PROGRAMS)

all:  $(HEADERS) $(DYNAMIC_LIB) $(STATIC_LIB) $(PROGRAMS)

eqs: $(HEADERS) $(DYNAMIC_LIB) $(STATIC_LIB)

include $(TOP)/make/rules.mk
