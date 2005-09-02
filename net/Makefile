
include $(TOP)/make/system.mk

HEADER_SRC = \
	base.h \
	commands.h \
	connection.h \
	connectionSet.h \
	connectionDescription.h \
	fdConnection.h \
	global.h \
	group.h \
	idHash.h \
	message.h \
	node.h \
	packet.h \
	pipeConnection.h \
	session.h \
	uniPipeConnection.h

HEADER_GEN = \
	node.h \
	session.h

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
	uniPipeConnection.cpp \
	user.cpp \
	util.cpp

MODULE  = net

LDFLAGS += -leqbase

all: $(HEADERS) $(STATIC_LIB)

include $(TOP)/make/rules.mk
