
include $(TOP)/make/system.mk

CXXFLAGS    += -I.. -I$(TOP)/server
SA_CXXFLAGS += -lpthread -leq -leqserver

ifeq "${ARCH}" "Linux"
SA_CXXFLAGS += -lrt -luuid
endif

TARGETS = $(SIMPLE_PROGRAMS) $(TESTS)

include $(TOP)/make/rules.mk

$(SIMPLE_PROGRAMS): $(wildcard $(TOP)/lib/client/image.*)