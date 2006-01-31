
include $(TOP)/make/system.mk

CXXFLAGS    += -I.. $(WINDOW_SYSTEM_INCS)
PROGRAMS     = flip
SA_CXXFLAGS += -L$(SAMPLE_LIB_DIR) -leq -lpthread $(WINDOW_SYSTEM_LIBS)

ifeq "${ARCH}" "Linux"
SA_CXXFLAGS += -lrt
endif

CLEAN = $(PROGRAMS)

all: $(PROGRAMS)

include $(TOP)/make/rules.mk
