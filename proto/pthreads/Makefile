
include $(TOP)/make/system.mk

PROGRAM = perf
SOURCES = perf.cpp

SA_LDFLAGS  += -L$(LIBRARY_DIR) -leq -lpthread

ifeq "${ARCH}" "Linux"
SA_LDFLAGS += -lrt
endif

TARGETS = $(PROGRAMS)

include $(TOP)/make/rules.mk
