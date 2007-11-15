
include $(TOP)/make/system.mk

PROGRAM = perf
SOURCES = perf.cpp
NOINSTALL = 1
BIN_DIR   = $(ARCH)

LDFLAGS  += -L$(LIBRARY_DIR) -leq -lpthread

ifeq "${ARCH}" "Linux"
  LDFLAGS += -lrt
ifeq "${SUBARCH}" "ia64"
    LDFLAGS += -luuid
endif
endif

TARGETS = $(PROGRAMS)

include $(TOP)/make/rules.mk
