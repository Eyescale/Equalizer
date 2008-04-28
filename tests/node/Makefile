
include $(TOP)/make/system.mk

CXXFLAGS += -I..
LDFLAGS  += -leq
BIN_DIR   = $(ARCH)

ifeq "${ARCH}" "Linux"
  LDFLAGS += -lrt
ifeq "${SUBARCH}" "ia64"
    LDFLAGS += -luuid
endif
endif

TARGETS = $(SIMPLE_PROGRAMS)

include $(TOP)/make/rules.mk
