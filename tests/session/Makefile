
include $(TOP)/make/system.mk

CXXFLAGS += -L$(BUILD_DIR)/lib -leqnet
PROGRAMS = init

CLEAN = $(PROGRAMS)

all: $(PROGRAMS)

init: init.cpp
	$(CXX) init.cpp $(CXXFLAGS) -o $@ 

include $(TOP)/make/rules.mk
