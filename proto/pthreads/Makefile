
include $(TOP)/make/system.mk

PROGRAMS      = perf
SA_CXXFLAGS  += -L$(SAMPLE_LIB_DIR) -leq -lpthread

ifeq "${ARCH}" "Linux"
SA_CXXFLAGS += -lrt
endif

CLEAN = $(PROGRAMS)

all: $(PROGRAMS)

include $(TOP)/make/rules.mk
