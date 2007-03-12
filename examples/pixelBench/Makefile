
include $(TOP)/make/system.mk

PROGRAM = pixelBench

SOURCES = \
	channel.cpp \
	config.cpp \
	configEvent.cpp \
	main.cpp

CXXFLAGS    += $(WINDOW_SYSTEM_INCS)
SA_LDFLAGS  += -leq -lpthread $(WINDOW_SYSTEM_LIBS)

ifeq "${ARCH}" "Linux"
  SA_LDFLAGS += -lrt
ifeq "${SUBARCH}" "ia64"
    SA_LDFLAGS += -luuid
endif
endif

TARGETS = $(PROGRAMS)

include $(TOP)/make/rules.mk
