
# location for top-level directory
ifndef TOP
  TOP := .
endif

SUBDIR    ?= "."
SUBDIRTOP := ../$(TOP)
DEPTH     := $(subst ../,--,$(TOP))
DEPTH     := $(subst .,-->,$(DEPTH))

# os-specific settings
ARCH    ?= $(subst .,_,$(subst -,_,$(shell uname)))
SUBARCH ?= $(shell uname -m)
RELARCH ?= $(shell uname -r)

include $(TOP)/make/$(ARCH).mk
-include $(TOP)/make/local.mk

# general variables, targets, etc.
VARIANTS           ?= $(SUBARCH)

BUILD_DIR_BASE  = build/$(ARCH)
BUILD_DIR       = $(TOP)/$(BUILD_DIR_BASE)
LIBRARY_DIR     = $(BUILD_DIR)/$(VARIANT)/lib
INCLUDEDIRS     = -I$(BUILD_DIR)/include
LINKDIRS        = -L$(LIBRARY_DIR)
BIN_DIR        ?= $(BUILD_DIR)/bin

WINDOW_SYSTEM_DEFINES = $(foreach WS,$(WINDOW_SYSTEM),-D$(WS))
DEP_CXX        ?= $(CXX)
DEFFLAGS       += -D$(ARCH) $(WINDOW_SYSTEM_DEFINES) -DEQ_CHECK_THREADSAFETY \
                  -DEQ_USE_COMPRESSION -DGLEW_MX


ifeq (0,${MAKELEVEL})
ifdef USE_OPENMP
    DEFFLAGS += -DEQ_USE_OPENMP
    CXXFLAGS += -fopenmp
    LDFLAGS  += -lgomp
endif
ifneq ($(findstring -g, $(CXXFLAGS)),-g)
    DEFFLAGS       += -DNDEBUG
ifneq ($(findstring -O, $(CXXFLAGS)),-O)
    CXXFLAGS       += -O2
endif # -O
endif # -g

  CFLAGS         += $(DEFFLAGS)
  CXXFLAGS       += $(DEFFLAGS)

ifeq ($(findstring g++, $(CXX)),g++)
    CXXFLAGS += -Wall \
                -Wnon-virtual-dtor -Wsign-promo -Wshadow \
                -Wno-unknown-pragmas -Wno-unused-parameter -Wno-write-strings
endif # g++
endif # top-level

export CFLAGS
export CXXFLAGS
export LDFLAGS

DOXYGEN        ?= Doxygen
FLEX           ?= flex
BISON          ?= bison

# defines
CXX_DEFINES      = $(sort $(filter -D%,$(CXXFLAGS)))
CXX_DEFINES_FILE = lib/base/defines.h
CXX_DEFINES_TXT  = $(CXX_DEFINES:-D%= %)

# include file variables
INCLUDE_BASE    = include/$(MODULE)
INCLUDE_DIR     = $(BUILD_DIR)/$(INCLUDE_BASE)
HEADERS         = $(HEADER_SRC:%=$(INCLUDE_DIR)/%)

# share files
SHARE_DIR       = $(BUILD_DIR)/share/Equalizer

# source code variables
SIMPLE_CXXFILES = $(wildcard *.cpp)
OBJECT_DIR      = $(TOP)/obj/$(SUBDIR)
OBJECT_SUFFIX   = $(ARCH).$(VARIANT)

ifndef VARIANT
  OBJECTS       = build_variants
else
  OBJECTS       = $(CXXFILES:%.cpp=$(OBJECT_DIR)/%.$(OBJECT_SUFFIX).o) \
		  $(CFILES:%.c=$(OBJECT_DIR)/%.$(OBJECT_SUFFIX).o)
  DEPENDENCIES  = $(OBJECTS:%=%.d) $(THIN_SIMPLE_PROGRAMS:%=%.d)
#  PCHEADERS     = $(HEADER_SRC:%=$(OBJECT_DIR)/%.gch)
endif

# library variables
LIBRARY           = $(DYNAMIC_LIB)
FAT_STATIC_LIB    = $(BUILD_DIR)/lib/lib$(MODULE).a
FAT_DYNAMIC_LIB   = $(BUILD_DIR)/lib/lib$(MODULE).$(DSO_SUFFIX)

ifdef VARIANT
THIN_STATIC_LIBS  = $(BUILD_DIR)/$(VARIANT)/lib/lib$(MODULE).a
THIN_DYNAMIC_LIBS = $(BUILD_DIR)/$(VARIANT)/lib/lib$(MODULE).$(DSO_SUFFIX)

else

THIN_STATIC_LIBS  = $(foreach V,$(VARIANTS),$(BUILD_DIR)/$(V)/lib/lib$(MODULE).a)
THIN_DYNAMIC_LIBS = $(foreach V,$(VARIANTS),$(BUILD_DIR)/$(V)/lib/lib$(MODULE).$(DSO_SUFFIX))
endif

# executable target
THIN_PROGRAMS     = $(foreach V,$(VARIANTS),$(BIN_DIR)/$(PROGRAM).$(V))
FAT_PROGRAM       = $(BIN_DIR)/$(PROGRAM)
APP_PROGRAM       = $(BIN_DIR)/$(PROGRAM).app/Contents/MacOS/$(PROGRAM)

FAT_SIMPLE_PROGRAMS  = $(SIMPLE_CXXFILES:%.cpp=$(BIN_DIR)/%)
THIN_SIMPLE_PROGRAMS = $(foreach V,$(VARIANTS),$(foreach P,$(FAT_SIMPLE_PROGRAMS),$(P).$(V)))
TESTS               ?= $(THIN_SIMPLE_PROGRAMS:%=%.testOk)

DYNAMIC_LIB       = $(THIN_DYNAMIC_LIBS)
STATIC_LIB        = $(THIN_STATIC_LIBS)
PROGRAMS         += $(THIN_PROGRAMS)
SIMPLE_PROGRAMS   = $(THIN_SIMPLE_PROGRAMS)

ifdef BUILD_FAT
DYNAMIC_LIB      += $(FAT_DYNAMIC_LIB)
STATIC_LIB       += $(FAT_STATIC_LIB)
PROGRAMS         += $(FAT_PROGRAM)
SIMPLE_PROGRAMS  += $(FAT_SIMPLE_PROGRAMS)
endif

# install variables
INSTALL_CMD     = $(TOP)/install.sh
INSTALL_FILES   = $(TOP)/install.files
