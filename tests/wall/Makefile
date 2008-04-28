
include $(TOP)/make/system.mk

CXXFLAGS += -I.. -I$(TOP)/server
LDFLAGS  += -lpthread -leq -leqserver
BIN_DIR   = $(ARCH)

ifeq "${ARCH}" "Linux"
LDFLAGS += -lrt -luuid
endif

TARGETS = $(SIMPLE_PROGRAMS) $(TESTS)

include $(TOP)/make/rules.mk

$(SIMPLE_PROGRAMS): $(wildcard $(TOP)/lib/client/image.*)