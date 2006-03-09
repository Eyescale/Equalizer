
include $(TOP)/make/system.mk

SA_CXXFLAGS += -I.. -L$(SAMPLE_LIB_DIR) -leqnet

ifeq "${ARCH}" "Linux"
SA_CXXFLAGS += -lrt
endif

TARGETS = $(SIMPLE_PROGRAMS)

include $(TOP)/make/rules.mk
