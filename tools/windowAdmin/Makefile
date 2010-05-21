
include $(TOP)/make/system.mk

PROGRAM = windowAdmin

CXXFILES = \
	main.cpp

CXXFLAGS += -I../../examples/include $(WINDOW_SYSTEM_INCS)
LDFLAGS  += -leqadmin -leq -lpthread $(WINDOW_SYSTEM_LIBS)

ifeq "${ARCH}" "Linux"
  LDFLAGS += -lrt
endif

TARGETS = $(PROGRAMS)

include $(TOP)/make/rules.mk
