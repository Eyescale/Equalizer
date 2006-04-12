
include $(TOP)/make/system.mk

SA_CXXFLAGS += -I.. -leq

ifeq "${ARCH}" "Linux"
SA_CXXFLAGS += -lrt
endif

TARGETS = $(SIMPLE_PROGRAMS)

include $(TOP)/make/rules.mk
