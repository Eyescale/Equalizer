
include $(TOP)/make/system.mk

CXXFLAGS    += -L$(BUILD_DIR)/lib -leqnet
PROGRAMS     = node
SA_CXXFLAGS += -lpthread

CLEAN = $(PROGRAMS)

all: $(PROGRAMS)

include $(TOP)/make/rules.mk
