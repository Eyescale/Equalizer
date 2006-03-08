
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
LDFLAGS     += -L$(LIBRARY_DIR) -leq
SA_CXXFLAGS += -L$(SAMPLE_LIB_DIR) -leq -leqserver

ifeq "${ARCH}" "Linux"
SA_CXXFLAGS += -lrt
endif

CLEAN = $(PROGRAMS)

all: $(PROGRAMS)

$(PROGRAMS): $(HEADERS) $(DYNAMIC_LIB)
$(STATIC_LIB): $(SAMPLE_LIB_DIR)/libeq.$(DSO_SUFFIX)
$(DYNAMIC_LIB): $(SAMPLE_LIB_DIR)/libeq.$(DSO_SUFFIX)

include $(TOP)/make/rules.mk
