
include $(TOP)/make/system.mk

CXXFLAGS    += -I.. $(WINDOW_SYSTEM_INCS)
SA_CXXFLAGS += -lpthread -leq

ifeq "${ARCH}" "Linux"
SA_CXXFLAGS += -lrt -luuid
endif

TARGETS = $(SIMPLE_PROGRAMS) $(TESTS)

include $(TOP)/make/rules.mk

$(SIMPLE_PROGRAMS): $(wildcard $(TOP)/lib/net/*Stream.*)
