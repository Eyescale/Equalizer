

include $(TOP)/make/system.mk

MODULE = eq/admin

HEADER_SRC = \
	admin.h \
	base.h \
	canvas.h \
	channel.h \
	client.h \
	init.h \
	layout.h \
	node.h \
	observer.h \
	pipe.h \
	segment.h \
	server.h \
	types.h \
	view.h \
	window.h \

CXXFILES = \
	canvas.cpp \
	channel.cpp \
	client.cpp \
	config.cpp \
	init.cpp \
	layout.cpp \
	node.cpp \
	nodeFactory.cpp \
	observer.cpp \
	pipe.cpp \
	segment.cpp \
	server.cpp \
	view.cpp \
	window.cpp \

CXXFLAGS += -DEQADMIN_EXPORTS
LDFLAGS += -leq

TARGETS = $(HEADERS) $(DYNAMIC_LIB)

include $(TOP)/make/rules.mk
