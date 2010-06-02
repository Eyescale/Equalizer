
include $(TOP)/make/system.mk

PROGRAM = eqPixelBench

CXXFILES = \
	channel.cpp \
	config.cpp \
	configEvent.cpp \
	main.cpp \
	window.cpp

CXXFLAGS += $(WINDOW_SYSTEM_INCS)
LDFLAGS  += -leq -lpthread $(WINDOW_SYSTEM_LIBS)

ifeq "${ARCH}" "Linux"
  LDFLAGS += -lrt
endif

TARGETS = $(PROGRAMS)

include $(TOP)/make/rules.mk
