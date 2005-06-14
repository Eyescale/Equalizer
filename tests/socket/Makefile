
include $(TOP)/make/system.mk

CXXFLAGS += -I$(TOP)/net -L$(BUILD_DIR)/lib -leqnet
PROGRAMS = client server

CLEAN = $(PROGRAMS)

all: $(PROGRAMS)

client: client.cpp
	$(CXX) client.cpp $(CXXFLAGS) -o $@ 

server: server.cpp
	$(CXX) server.cpp $(CXXFLAGS) -o $@

include $(TOP)/make/rules.mk
