
include $(TOP)/make/system.mk

CXXFLAGS += -I.. $(WINDOW_SYSTEM_INCS)
LDFLAGS  += -lpthread -leq
BIN_DIR   = $(ARCH)

ifeq "${ARCH}" "Linux"
LDFLAGS += -lrt -luuid
endif

TARGETS = #$(SIMPLE_PROGRAMS)

include $(TOP)/make/rules.mk
