
include $(TOP)/make/system.mk

CXXFLAGS += -I..
LDFLAGS  += -lpthread
BIN_DIR   = $(ARCH)

TARGETS = $(SIMPLE_PROGRAMS) $(TESTS)

include $(TOP)/make/rules.mk

$(SIMPLE_PROGRAMS): $(wildcard $(TOP)/lib/client/image.*)