
include $(TOP)/make/system.mk

CXXFLAGS += -I..
LDFLAGS  += -lpthread -lEqualizer
BIN_DIR   = $(ARCH)

ifeq "${ARCH}" "Linux"
LDFLAGS += -lrt
endif

TARGETS = $(SIMPLE_PROGRAMS) $(TESTS)

include $(TOP)/make/rules.mk
