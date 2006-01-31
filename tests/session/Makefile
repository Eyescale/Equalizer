
include $(TOP)/make/system.mk

CXXFLAGS    += -I.. $(WINDOW_SYSTEM_INCS)
SA_CXXFLAGS += -lpthread -L$(SAMPLE_LIB_DIR) -leqnet

ifeq "${ARCH}" "Linux"
SA_CXXFLAGS += -lrt
endif

CLEAN = $(SIMPLE_PROGRAMS)

all: $(SIMPLE_PROGRAMS)

include $(TOP)/make/rules.mk
