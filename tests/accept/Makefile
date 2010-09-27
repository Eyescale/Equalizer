
include $(TOP)/make/system.mk

CXXFLAGS += -I.. -I$(TOP)/examples/include
LDFLAGS  += -lEqualizer -lpthread

ifeq "${ARCH}" "Linux"
LDFLAGS += -lrt
endif

TARGETS = $(SIMPLE_PROGRAMS)

include $(TOP)/make/rules.mk
