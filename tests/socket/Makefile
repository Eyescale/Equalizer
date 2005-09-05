
include $(TOP)/make/system.mk

CXXFLAGS += -L$(SAMPLE_LIB_DIR) -leqnet
PROGRAMS = client server pipe

CLEAN = $(PROGRAMS)

all: $(PROGRAMS)

client: client.cpp
	$(CXX) client.cpp $(CXXFLAGS) -o $@ 

server: server.cpp
	$(CXX) server.cpp $(CXXFLAGS) -o $@

pipe: pipe.cpp
	$(CXX) pipe.cpp $(CXXFLAGS) -o $@

include $(TOP)/make/rules.mk
