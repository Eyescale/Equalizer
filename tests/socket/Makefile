
include $(TOP)/make/system.mk

CXXFLAGS += -I$(TOP)/net -L$(BUILD_DIR)/lib -leqnet

CLEAN = $(wildcard *.o) client server

all: client server

client: client.cxx
	$(CXX) $(CXXFLAGS) -o $@ client.cxx

server: server.cxx
	$(CXX) $(CXXFLAGS) -o $@ server.cxx

include $(TOP)/make/rules.mk
