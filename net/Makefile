
include $(TOP)/make/system.mk

HEADER_SRC = \
	connectionDescription.h \
	global.h \
	group.h \
	network.h \
	node.h \
	session.h

SOURCES = \
	connection.cpp \
	fdConnection.cpp \
	pipeConnection.cpp \
	session.cpp \
	server.cpp \
	socketConnection.cpp

MODULE  = net

LDFLAGS += -leqbase

all: $(HEADERS) $(LIBRARY)

include $(TOP)/make/rules.mk
