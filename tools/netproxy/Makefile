
include $(TOP)/make/system.mk

CXXFLAGS += -I.. -I$(TOP)/examples/include
LDFLAGS  += -leq -lpthread

ifeq "${ARCH}" "Linux"
LDFLAGS += -lrt -luuid
endif

TARGETS = $(SIMPLE_PROGRAMS)

include $(TOP)/make/rules.mk
