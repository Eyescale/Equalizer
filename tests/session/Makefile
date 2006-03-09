
include $(TOP)/make/system.mk

CXXFLAGS    += -I.. $(WINDOW_SYSTEM_INCS)
SA_CXXFLAGS += -lpthread -L$(SAMPLE_LIB_DIR) -leq

ifeq "${ARCH}" "Linux"
SA_CXXFLAGS += -lrt -luuid
endif

CLEAN = $(SIMPLE_PROGRAMS)

TARGETS = $(SIMPLE_PROGRAMS)

include $(TOP)/make/rules.mk
