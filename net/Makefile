
include $(TOP)/make/system.mk

HEADER_SRC = \
	base.h \
	commands.h \
	connection.h \
	connectionSet.h \
	connectionDescription.h \
	global.h \
	group.h \
	message.h \
	node.h \
	packet.h \
	session.h

HEADER_GEN = \
#	sessionPriv.h

SOURCES = \
	base.cpp \
	connection.cpp \
	connectionSet.cpp \
	fdConnection.cpp \
	global.cpp \
	launcher.cpp \
	node.cpp \
	pipeConnection.cpp \
	session.cpp \
	socketConnection.cpp \
	user.cpp \
	util.cpp

MODULE  = net

LDFLAGS += -leqbase

all: $(HEADERS) $(LIBRARY)

include $(TOP)/make/rules.mk
