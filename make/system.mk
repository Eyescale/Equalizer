
ifndef TOP
  TOP := .
endif

SUBTOP := ../$(TOP)
DEPTH := $(subst ../,-,$(TOP))
DEPTH := $(subst .,->,$(DEPTH))

include $(TOP)/make/Darwin.mk

CXXFLAGS += -I$(BUILD_DIR)/include

BUILD_DIR  = $(TOP)/build/$(ARCH)/$(VARIANT)

HEADER_SRC = $(wildcard *.h)
HEADER_DIR = $(BUILD_DIR)/include/eq/$(MODULE)
HEADERS    = $(HEADER_SRC:%=$(HEADER_DIR)/%)

OBJECT_DIR   = obj/$(ARCH)/$(VARIANT)
OBJECTS      = $(SOURCES:%.cpp=$(OBJECT_DIR)/%.o)

LIBRARY_DIR = $(BUILD_DIR)/lib
LIBRARY     = $(MODULE:%=$(LIBRARY_DIR)/libeq%.$(DSO_SUFFIX))

DEPENDENCIES = $(OBJECTS:%.o=%.d)


