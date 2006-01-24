
include $(TOP)/make/system.mk

HEADER_SRC = \
	barrier.h \
	base.h \
	commands.h \
	connection.h \
	connectionSet.h \
	connectionDescription.h \
	fdConnection.h \
	global.h \
	group.h \
	idHash.h \
	init.h \
	message.h \
	node.h \
	object.h \
	packets.h \
	pipeConnection.h \
	requestQueue.h \
	session.h \
	uniPipeConnection.h

SOURCES = \
	barrier.cpp \
	base.cpp \
	connection.cpp \
	connectionDescription.cpp \
	connectionSet.cpp \
	fdConnection.cpp \
	global.cpp \
	init.cpp \
	launcher.cpp \
	node.cpp \
	object.cpp \
	pipeConnection.cpp \
	requestQueue.cpp \
	session.cpp \
	socketConnection.cpp \
	uniPipeConnection.cpp \
	util.cpp

MODULE  = net

LDFLAGS += -leqbase

all: $(HEADERS) $(STATIC_LIB)

$(STATIC_LIB): $(SAMPLE_LIB_DIR)/libeqbase.a
$(DYNAMIC_LIB): $(SAMPLE_LIB_DIR)/libeqbase.a

include $(TOP)/make/rules.mk
