
include $(TOP)/make/system.mk

HEADER_SRC = \
	channel.h \
	commands.h \
	config.h \
	configParams.h \
	eq.h \
	frustum.h \
	global.h \
	init.h \
	node.h \
	nodeFactory.h \
	packets.h \
	pipe.h \
	pixelViewport.h \
	projection.h \
	renderContext.h \
	server.h \
	viewport.h \
	wall.h \
	window.h

SOURCES = \
	channel.cpp \
	client.cpp \
	config.cpp \
	configParams.cpp \
	global.cpp \
	init.cpp \
	node.cpp \
	pipe.cpp \
	server.cpp \
	window.cpp

CXXFLAGS += $(WINDOW_SYSTEM_INCS)
LDFLAGS  += -leqnet $(WINDOW_SYSTEM_LIBS)

# ifeq "${ARCH}" "Linux"
# LDFLAGS += -lrt
# endif

all: $(HEADERS) $(DYNAMIC_LIB)

$(STATIC_LIB): $(SAMPLE_LIB_DIR)/libeqnet.a
$(DYNAMIC_LIB): $(SAMPLE_LIB_DIR)/libeqnet.a

include $(TOP)/make/rules.mk
