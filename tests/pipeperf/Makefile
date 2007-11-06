
include $(TOP)/make/system.mk

CXXFLAGS += -I.. -leq -lpthread
BIN_DIR   = $(ARCH)

ifeq "${ARCH}" "Linux"
SA_CXXFLAGS += -lrt -luuid
endif

TARGETS = $(SIMPLE_PROGRAMS) $(TESTS)

include $(TOP)/make/rules.mk
