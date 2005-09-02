
include $(TOP)/make/system.mk

CXXFLAGS     += -L$(LIBRARY_DIR) -leqbase
PROGRAMS      = perf
SA_CXXFLAGS  += -lpthread

CLEAN = $(PROGRAMS)

all: $(PROGRAMS)

include $(TOP)/make/rules.mk
