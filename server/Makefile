
include $(TOP)/make/system.mk

CXXFLAGS += -L$(BUILD_DIR)/lib -leqnet
PROGRAMS = eqs

CLEAN = $(PROGRAMS)

all: $(PROGRAMS)

init: eqs.cpp
	$(CXX) eqs.cpp $(CXXFLAGS) -o $@ 

include $(TOP)/make/rules.mk
