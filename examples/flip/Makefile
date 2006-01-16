
include $(TOP)/make/system.mk

CXXFLAGS    += -I.. -L$(SAMPLE_LIB_DIR) -leq
PROGRAMS     = flip
SA_CXXFLAGS += -lpthread $(WINDOW_SYSTEM_LIBS)

ifeq "${ARCH}" "Linux"
SA_CXXFLAGS += -lrt
endif

CLEAN = $(PROGRAMS)

all: $(PROGRAMS)

include $(TOP)/make/rules.mk
