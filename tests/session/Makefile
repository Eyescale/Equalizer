
include $(TOP)/make/system.mk

CXXFLAGS    += -L$(BUILD_DIR)/lib -leqnet
PROGRAMS     = init
SA_CXXFLAGS += -lpthread

CLEAN = $(PROGRAMS)

all: $(PROGRAMS)

include $(TOP)/make/rules.mk
