
include $(TOP)/make/system.mk

CXXFLAGS += -I$(TOP)/net -L$(BUILD_DIR)/lib -leqnet

CLEAN = client server

all: client server

client: client.cxx
	$(CXX) client.cxx $(CXXFLAGS) -o $@ 

server: server.cxx
	$(CXX) server.cxx $(CXXFLAGS) -o $@

include $(TOP)/make/rules.mk
