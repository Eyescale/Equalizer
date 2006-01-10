
include $(TOP)/make/system.mk

CXXFLAGS += -I.. -L$(SAMPLE_LIB_DIR) -leqnet

ifeq "${ARCH}" "Linux"
SA_CXXFLAGS += -lrt
endif

CLEAN = $(PROGRAMS)

all: $(PROGRAMS)

include $(TOP)/make/rules.mk
