
include $(TOP)/make/system.mk

PROGRAM = configTool

CXXFILES = \
	configTool.cpp \
	writeFromFile.cpp

CXXFLAGS += -I../../examples/include -I../.. $(WINDOW_SYSTEM_INCS) 
LDFLAGS  += $(WINDOW_SYSTEM_LIBS) -leq -leqserver

TARGETS = $(PROGRAMS)

include $(TOP)/make/rules.mk
