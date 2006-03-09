
include $(TOP)/make/system.mk

CXXFLAGS += -I.. -L$(SAMPLE_LIB_DIR) -leq -lpthread

ifeq "${ARCH}" "Linux"
SA_CXXFLAGS += -lrt -luuid
endif

TARGETS = $(SIMPLE_PROGRAMS)

include $(TOP)/make/rules.mk
