
include $(TOP)/make/system.mk

CXXFLAGS    += -I.. $(WINDOW_SYSTEM_INCS)
SA_CXXFLAGS += -lpthread -leq
BIN_DIR      = $(ARCH)

ifeq "${ARCH}" "Linux"
SA_CXXFLAGS += -lrt -luuid
endif

TARGETS = #$(SIMPLE_PROGRAMS)

include $(TOP)/make/rules.mk
