
include $(TOP)/make/system.mk

PROGRAM = perf
SOURCES = perf.cpp
NOINSTALL = 1

SA_LDFLAGS  += -L$(LIBRARY_DIR) -leq -lpthread

ifeq "${ARCH}" "Linux"
  SA_LDFLAGS += -lrt
ifeq "${SUBARCH}" "ia64"
    SA_LDFLAGS += -luuid
endif
endif

TARGETS = $(PROGRAMS)

include $(TOP)/make/rules.mk
