
include $(TOP)/make/system.mk

CXXFLAGS += -I.. $(WINDOW_SYSTEM_INCS) -I$(TOP)/lib
LDFLAGS  += -lpthread -lEqualizer
BIN_DIR   = $(ARCH)

ifeq "${ARCH}" "Linux"
LDFLAGS += -lrt -luuid
endif

TARGETS = $(SIMPLE_PROGRAMS) $(TESTS)

include $(TOP)/make/rules.mk

$(SIMPLE_PROGRAMS): $(wildcard $(TOP)/lib/util/gpuCompressor.*) $(wildcard $(TOP)/lib/base/pluginRegistry.*) $(wildcard $(TOP)/lib/base/plugin.*) $(wildcard $(TOP)/lib/base/dso.*) $(wildcard $(TOP)/lib/base/cpuCompressor.*) $(wildcard $(TOP)/lib/base/compressor.*) $(wildcard $(TOP)/lib/compressor/*)