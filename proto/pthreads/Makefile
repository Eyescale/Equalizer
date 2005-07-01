
include $(TOP)/make/system.mk

CXXFLAGS += -L$(BUILD_DIR)/lib -leqbase
PROGRAMS = perf

CLEAN = $(PROGRAMS)

all: $(PROGRAMS)

perf: perf.cpp
	$(CXX) perf.cpp $(CXXFLAGS) -o $@ 

include $(TOP)/make/rules.mk
