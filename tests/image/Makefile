
include $(TOP)/make/system.mk

CXXFLAGS += -I.. $(WINDOW_SYSTEM_INCS) -I$(TOP)/lib
LDFLAGS  += -lpthread -lEqualizer
BIN_DIR   = $(ARCH)

ifeq "${ARCH}" "Linux"
LDFLAGS += -lrt
endif

TARGETS = $(SIMPLE_PROGRAMS) $(TESTS)

include $(TOP)/make/rules.mk

$(SIMPLE_PROGRAMS): $(wildcard $(TOP)/lib/client/image.*) $(wildcard $(TOP)/lib/compressor/*)