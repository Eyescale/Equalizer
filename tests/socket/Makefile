
TOP = ../..
include $(TOP)/make/system.mk

CXXFLAGS += -I$(TOP)/net -L$(TOP)/lib -leqnet

all: client

client: client.cxx
	$(CXX) $(CXXFLAGS) -o $@ client.cxx

server: server.cxx
	$(CXX) $(CXXFLAGS) -o $@ server.cxx

include $(TOP)/make/rules.mk
