
# location for top-level directory
ifndef TOP
  TOP := .
endif

SUBDIRTOP := ../$(TOP)
DEPTH     := $(subst ../,-,$(TOP))
DEPTH     := $(subst .,->,$(DEPTH))

# os-specific settings
ARCH = $(shell uname)
include $(TOP)/make/$(ARCH).mk

# general variables, targets, etc.
BUILD_DIR       = $(TOP)/build/$(ARCH)/$(VARIANT)

CXXFLAGS       += -I$(BUILD_DIR)/include -D$(ARCH)
LDFLAGS        += -L$(BUILD_DIR)/lib
CXX_DEPS       ?= $(CXX)
CXX_DEPS_FLAGS  = -I$(BUILD_DIR)/include -D$(ARCH)
DOXYGEN        ?= Doxygen

# header file variables
HEADER_SRC      = $(wildcard *.h)
HEADER_DIR      = $(BUILD_DIR)/include/eq/$(MODULE)
HEADERS         = $(HEADER_SRC:%=$(HEADER_DIR)/%)

# source code variables
CPPFILES        = $(wildcard *.cpp)
OBJECT_DIR      = obj/$(ARCH)/$(VARIANT)
OBJECTS         = $(SOURCES:%.cpp=$(OBJECT_DIR)/%.o)

# library variables
LIBRARY_DIR     = $(BUILD_DIR)/lib
LIBRARY         = $(DYNAMIC_LIB)
STATIC_LIB      = $(MODULE:%=$(LIBRARY_DIR)/libeq%.a)
DYNAMIC_LIB     = $(MODULE:%=$(LIBRARY_DIR)/libeq%.$(DSO_SUFFIX))

SIMPLE_PROGRAMS = $(CPPFILES:%.cpp=%)

DEPENDENCIES ?= $(OBJECTS:%.o=%.d)

