
include $(TOP)/make/system.mk

HEADER_SRC = \
	base.h \
	connectionDescription.h \
	global.h \
	group.h \
	message.h \
	network.h \
	node.h \
	server.h \
	session.h

SOURCES = \
	connection.cpp \
	connectionListener.cpp \
	connectionNetwork.cpp \
	connectionSet.cpp \
	fdConnection.cpp \
	global.cpp \
	launcher.cpp \
	network.cpp \
	networkPriv.cpp \
	node.cpp \
	nodePriv.cpp \
	pipeConnection.cpp \
	pipeNetwork.cpp \
	session.cpp \
	sessionPriv.cpp \
	server.cpp \
	serverPriv.cpp \
	socketConnection.cpp \
	socketNetwork.cpp \
	util.cpp

MODULE  = net

LDFLAGS += -leqbase

all: $(HEADERS) $(LIBRARY)

include $(TOP)/make/rules.mk
