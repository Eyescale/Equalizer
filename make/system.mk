
ifndef TOP
  TOP := .
endif

SUBTOP := ../$(TOP)

include $(TOP)/make/Darwin.mk

CXXFLAGS += -I$(BUILD_DIR)/include

BUILD_DIR  = $(TOP)/build/$(ARCH)/$(VARIANT)

HEADER_SRC = $(wildcard *.h)
HEADER_DIR = $(BUILD_DIR)/include/eq/$(MODULE_NAME)
HEADERS    = $(HEADER_SRC:%=$(HEADER_DIR)/%)

OBJECT_DIR   = obj/$(ARCH)/$(VARIANT)
OBJECTS      = $(SOURCES:%.cpp=$(OBJECT_DIR)/%.o)

LIBRARY_DIR = $(BUILD_DIR)/lib
LIBRARY     = $(MODULE_NAME:%=$(LIBRARY_DIR)/libeq%.$(DSO_SUFFIX))


