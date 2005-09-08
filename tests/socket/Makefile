
include $(TOP)/make/system.mk

CXXFLAGS += -L$(SAMPLE_LIB_DIR) -leqnet
PROGRAMS = client server pipe

CLEAN = $(PROGRAMS)

all: $(PROGRAMS)

include $(TOP)/make/rules.mk
