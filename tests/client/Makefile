
include $(TOP)/make/system.mk

CXXFLAGS += -I.. $(WINDOW_SYSTEM_INCS)
LDFLAGS  += -leq -lpthread
BIN_DIR   = $(ARCH)

ifeq "${ARCH}" "Linux"
LDFLAGS += -lrt
endif

# test with obj-c++ compiler
ifeq ($(findstring AGL, $(WINDOW_SYSTEM)),AGL)
SIMPLE_PROGRAMS += $(BIN_DIR)/configVisitor_objc
endif


TARGETS = $(SIMPLE_PROGRAMS) $(TESTS)

include $(TOP)/make/rules.mk

$(SIMPLE_PROGRAMS): $(wildcard $(TOP)/lib/client/*)

configVisitor_objc.mm: configVisitor.cpp
	@cp $< $@
