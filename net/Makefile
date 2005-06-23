
include $(TOP)/make/system.mk

HEADER_SRC = \
	connectionDescription.h \
	global.h \
	group.h \
	message.h \
	network.h \
	node.h \
	session.h

SOURCES = \
	connection.cpp \
	fdConnection.cpp \
	network.cpp \
	networkPriv.cpp \
	node.cpp \
	nodePriv.cpp \
	pipeConnection.cpp \
	pipeNetwork.cpp \
	session.cpp \
	sessionPriv.cpp \
	server.cpp \
	socketConnection.cpp \
	socketNetwork.cpp

MODULE  = net

LDFLAGS += -leqbase

all: $(HEADERS) $(LIBRARY)

include $(TOP)/make/rules.mk
