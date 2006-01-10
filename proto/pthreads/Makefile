
include $(TOP)/make/system.mk

CXXFLAGS     += -L$(SAMPLE_LIB_DIR) -leqbase
PROGRAMS      = perf
SA_CXXFLAGS  += -lpthread

ifeq "${ARCH}" "Linux"
SA_CXXFLAGS += -lrt
endif

CLEAN = $(PROGRAMS)

all: $(PROGRAMS)

include $(TOP)/make/rules.mk
