
include $(TOP)/make/system.mk

PROGRAM = eqHello

CXXFILES = \
	hello.cpp

CXXFLAGS += -I../include $(WINDOW_SYSTEM_INCS)
LDFLAGS  += -lEqualizer -lpthread $(WINDOW_SYSTEM_LIBS)

ifeq "${ARCH}" "Linux"
  LDFLAGS += -lrt
endif

TARGETS = $(PROGRAMS)

include $(TOP)/make/rules.mk
