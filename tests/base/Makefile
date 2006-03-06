
include $(TOP)/make/system.mk

CXXFLAGS += -I.. -L$(SAMPLE_LIB_DIR) -leqbase -lpthread

ifeq "${ARCH}" "Linux"
SA_CXXFLAGS += -lrt -luuid
endif

CLEAN = $(SIMPLE_PROGRAMS)

all: $(SIMPLE_PROGRAMS)

include $(TOP)/make/rules.mk
