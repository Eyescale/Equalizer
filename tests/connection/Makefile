
include $(TOP)/make/system.mk

CXXFLAGS += -I..
LDFLAGS  += -leq -lpthread
BIN_DIR   = $(ARCH)

ifeq "${ARCH}" "Linux"
LDFLAGS += -lrt
endif

TARGETS = $(SIMPLE_PROGRAMS)

include $(TOP)/make/rules.mk
