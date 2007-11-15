
include $(TOP)/make/system.mk

PROGRAM = eqPixelBench

SOURCES = \
	channel.cpp \
	config.cpp \
	configEvent.cpp \
	main.cpp

CXXFLAGS += $(WINDOW_SYSTEM_INCS)
LDFLAGS  += -leq -lpthread $(WINDOW_SYSTEM_LIBS)

ifeq "${ARCH}" "Linux"
  LDFLAGS += -lrt
ifeq "${SUBARCH}" "ia64"
    LDFLAGS += -luuid
endif
endif

TARGETS = $(PROGRAMS)

include $(TOP)/make/rules.mk
