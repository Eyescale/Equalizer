
include $(TOP)/make/system.mk

CXXFLAGS += -I.. -leq -lpthread

ifeq "${ARCH}" "Linux"
SA_CXXFLAGS += -lrt -luuid
endif

TARGETS = $(SIMPLE_PROGRAMS) $(TESTS)

include $(TOP)/make/rules.mk
