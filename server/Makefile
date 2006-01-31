
include $(TOP)/make/system.mk

PROGRAMS   = eqs
HEADER_SRC = 

SOURCES = \
	channel.cpp \
	config.cpp \
	compound.cpp \
	node.cpp \
	pipe.cpp \
	server.cpp \
	window.cpp

MODULE  = server

CXXFLAGS    += $(WINDOW_SYSTEM_INCS)
LDFLAGS     += -L$(LIBRARY_DIR) -leqnet 
SA_CXXFLAGS += -L$(SAMPLE_LIB_DIR) -leqserver

ifeq "${ARCH}" "Linux"
SA_CXXFLAGS += -lrt
endif


CLEAN = $(PROGRAMS)

all: $(PROGRAMS)

$(PROGRAMS): $(HEADERS) $(DYNAMIC_LIB)
$(STATIC_LIB): $(SAMPLE_LIB_DIR)/libeqnet.a
$(DYNAMIC_LIB): $(SAMPLE_LIB_DIR)/libeqnet.a

include $(TOP)/make/rules.mk
