
include $(TOP)/make/system.mk

PROGRAM = configTool

SOURCES = \
	configTool.cpp

CXXFLAGS += -I../../examples/include $(WINDOW_SYSTEM_INCS)
SA_LDFLAGS  += -leq -lpthread $(WINDOW_SYSTEM_LIBS)

TARGETS = $(PROGRAMS)

include $(TOP)/make/rules.mk
