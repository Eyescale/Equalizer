
include $(TOP)/make/system.mk

PROGRAM = flip

SOURCES = \
	channel.cpp \
	flip.cpp

CXXFLAGS    += -I.. $(WINDOW_SYSTEM_INCS)
SA_LDFLAGS  += -L$(SAMPLE_LIB_DIR) -leq -lpthread $(WINDOW_SYSTEM_LIBS)

ifeq "${ARCH}" "Linux"
SA_LDFLAGS += -lrt
endif

TARGETS = $(PROGRAMS)

include $(TOP)/make/rules.mk
