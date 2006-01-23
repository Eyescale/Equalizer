
include $(TOP)/make/system.mk

SOURCES = \
	channel.cpp \
	client.cpp \
	config.cpp \
	configParams.cpp \
	global.cpp \
	init.cpp \
	node.cpp \
	openParams.cpp \
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
