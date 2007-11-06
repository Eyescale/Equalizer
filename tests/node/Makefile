
include $(TOP)/make/system.mk

SA_CXXFLAGS += -I.. -leq
BIN_DIR      = $(ARCH)

ifeq "${ARCH}" "Linux"
  SA_CXXFLAGS += -lrt
ifeq "${SUBARCH}" "ia64"
    SA_CXXFLAGS += -luuid
endif
endif

TARGETS = $(SIMPLE_PROGRAMS)

include $(TOP)/make/rules.mk
