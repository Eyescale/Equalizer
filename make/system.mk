
# location for top-level directory
ifndef TOP
  TOP := .
endif

SUBDIR    ?= "src"
SUBDIRTOP := ../$(TOP)
DEPTH     := $(subst ../,--,$(TOP))
DEPTH     := $(subst .,----->,$(DEPTH))

# os-specific settings
ARCH = $(shell uname)
include $(TOP)/make/$(ARCH).mk

# general variables, targets, etc.
BUILD_DIR       = $(TOP)/build/$(ARCH)
EXTRAS_DIR      = $(TOP)/extras
LIBRARY_DIR     = $(BUILD_DIR)/$(VARIANT)/lib
SAMPLE_LIB_DIR  = $(BUILD_DIR)/$(VARIANT1)/lib

INT_CXXFLAGS   += -I$(BUILD_DIR)/include -I$(EXTRAS_DIR) -DSUBDIR=\"$(SUBDIR)\"
INT_LDFLAGS    += -L$(LIBRARY_DIR)
DEP_CXX        ?= $(CXX)
DOXYGEN        ?= Doxygen

# defines
CXX_DEFINES           += -D$(ARCH) $(WINDOW_SYSTEM_DEFINES)
CXX_DEFINES           += -DCHECK_THREADSAFETY
WINDOW_SYSTEM_DEFINES  = $(foreach WS,$(WINDOW_SYSTEM),-D$(WS))
CXX_DEFINES_FILE       = lib/base/defines.h
CXX_DEFINES_TXT        = $(CXX_DEFINES:-D%= %)

# header file variables
HEADER_DIR      = $(BUILD_DIR)/include/eq/$(MODULE)
HEADERS         = $(HEADER_SRC:%=$(HEADER_DIR)/%)

# source code variables
CXXFILES        = $(wildcard *.cpp)
OBJECT_DIR      = obj/$(ARCH)/$(VARIANT)

ifndef VARIANT
  OBJECTS       = foo
  VARIANT1      = $(word 1, $(VARIANTS))
else
  OBJECTS       = $(SOURCES:%.cpp=$(OBJECT_DIR)/%.o)
  VARIANT1      = $(VARIANT)
endif

# library variables
LIBRARY         = $(DYNAMIC_LIB)
STATIC_LIB      = $(foreach V,$(VARIANTS),$(BUILD_DIR)/$(V)/lib/libeq$(MODULE).a)
DYNAMIC_LIB     = $(foreach V,$(VARIANTS),$(BUILD_DIR)/$(V)/lib/libeq$(MODULE).$(DSO_SUFFIX))
PROGRAMS        = $(foreach V,$(VARIANTS),$(PROGRAM).$(V))

SIMPLE_PROGRAMS = $(CXXFILES:%.cpp=%)

DEPENDENCIES   ?= $(OBJECTS:%.o=%.d)

