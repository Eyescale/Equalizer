
include $(TOP)/make/Darwin.mk

CXXFLAGS += -I$(TOP)/include

HEADER_SRC = $(wildcard *.h)
HEADER_DIR = $(TOP)/include/eq/$(MODULE_NAME)
HEADERS    = $(HEADER_SRC:%=$(HEADER_DIR)/%)

LIBRARY_DIR = $(TOP)/lib
LIBRARY     = $(MODULE_NAME:%=$(LIBRARY_DIR)/libeq%.$(DSO_SUFFIX))

