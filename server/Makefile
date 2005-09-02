
include $(TOP)/make/system.mk

PROGRAMS   = eqs
HEADER_SRC = \
	server.h

SOURCES = \
	server.cpp

MODULE  = server

LDFLAGS  += -L$(BUILD_DIR)/lib -leqnet
CLEAN = $(PROGRAMS)

all:  $(HEADERS) $(STATIC_LIB) $(PROGRAMS)

eqs: $(HEADERS) $(STATIC_LIB)

include $(TOP)/make/rules.mk
