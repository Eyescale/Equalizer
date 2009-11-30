
include $(TOP)/make/system.mk

CXXFLAGS += -I.. $(WINDOW_SYSTEM_INCS)
LDFLAGS  += -lpthread -leq -leqserver
BIN_DIR   = $(ARCH)

ifeq "${ARCH}" "Linux"
LDFLAGS += -lrt -luuid
endif

TARGETS = $(SIMPLE_PROGRAMS) $(TESTS)

include $(TOP)/make/rules.mk

$(SIMPLE_PROGRAMS): $(wildcard $(TOP)/server/loader.*)
