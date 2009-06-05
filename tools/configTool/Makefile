
include $(TOP)/make/system.mk

PROGRAM = configTool

CXXFILES = \
	configTool.cpp \
	writeFromFile.cpp

CXXFLAGS += -I../../examples/include -I../.. $(WINDOW_SYSTEM_INCS) 
LDFLAGS  += $(WINDOW_SYSTEM_LIBS) -leq -leqserver

ifeq "${ARCH}" "Linux"
  LDFLAGS += -lrt
ifeq "${SUBARCH}" "ia64"
    LDFLAGS += -luuid
endif
endif

TARGETS = $(PROGRAMS)

include $(TOP)/make/rules.mk
