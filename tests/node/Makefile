
include $(TOP)/make/system.mk

CXXFLAGS    += -I.. -L$(SAMPLE_LIB_DIR) -leqnet
PROGRAMS     = node
SA_CXXFLAGS += -lpthread

CLEAN = $(PROGRAMS)

all: $(PROGRAMS)

include $(TOP)/make/rules.mk
