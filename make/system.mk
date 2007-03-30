
# location for top-level directory
ifndef TOP
  TOP := .
endif

SUBDIR    ?= "."
SUBDIRTOP := ../$(TOP)
DEPTH     := $(subst ../,--,$(TOP))
DEPTH     := $(subst .,-->,$(DEPTH))

# os-specific settings
ARCH    = $(shell uname)
SUBARCH = $(shell uname -m)

include $(TOP)/make/$(ARCH).mk

# general variables, targets, etc.
VARIANTS       ?= $(SUBARCH)

INSTALL_DIR    ?= /usr/local
INSTALL_LIB_DIR    ?= $(INSTALL_DIR)/lib$(VARIANT)
INSTALL_BIN_DIR    ?= $(INSTALL_DIR)/bin
INSTALL_SHARE_DIR  ?= $(INSTALL_DIR)/share/Equalizer
INCLUDE_BASE        = include/eq/$(MODULE)
INSTALL_INCLUDE_DIR = $(INSTALL_DIR)/$(INCLUDE_BASE)

BUILD_DIR_BASE  = build/$(ARCH)
BUILD_DIR       = $(TOP)/$(BUILD_DIR_BASE)
LIBRARY_DIR     = $(BUILD_DIR)/$(VARIANT)/lib
INCLUDEDIRS     = -I$(BUILD_DIR)/include
LINKDIRS        = -L$(LIBRARY_DIR)

WINDOW_SYSTEM_DEFINES = $(foreach WS,$(WINDOW_SYSTEM),-D$(WS))
DEP_CXX        ?= $(CXX)

ifeq (0,${MAKELEVEL})
  CXXFLAGS       += -D$(ARCH) $(WINDOW_SYSTEM_DEFINES) -DEQ_CHECK_THREADSAFETY \
                    -DEQ_USE_COMPRESSION
ifneq ($(findstring -g, $(CXXFLAGS)),-g)
    CXXFLAGS       += -DNDEBUG
ifneq ($(findstring -O, $(CXXFLAGS)),-O)
    CXXFLAGS       += -O2
endif # -O
endif # -g
ifeq ($(CXX),g++)
    CXXFLAGS       += -Wall
endif # g++
endif # top-level

export CXXFLAGS

DOXYGEN        ?= Doxygen
FLEX           ?= flex
BISON          ?= bison

# defines
CXX_DEFINES      = $(sort $(filter -D%,$(CXXFLAGS)))
CXX_DEFINES_FILE = lib/base/defines.h
CXX_DEFINES_TXT  = $(CXX_DEFINES:-D%= %)

# include file variables
INCLUDE_DIR     = $(BUILD_DIR)/$(INCLUDE_BASE)
HEADERS         = $(HEADER_SRC:%=$(INCLUDE_DIR)/%)

# share files
SHARE_DIR       = $(BUILD_DIR)/share

# source code variables
CXXFILES        = $(wildcard *.cpp)
OBJECT_DIR      = obj/$(ARCH)/$(VARIANT)

ifndef VARIANT
  OBJECTS       = build_variants
else
  OBJECTS       = $(SOURCES:%.cpp=$(OBJECT_DIR)/%.o)
  DEPENDENCIES  = $(OBJECTS:%=%.d) $(THIN_SIMPLE_PROGRAMS:%=%.d)
#  PCHEADERS     = $(HEADER_SRC:%=$(OBJECT_DIR)/%.gch)
endif

# library variables
LIBRARY           = $(DYNAMIC_LIB)
FAT_STATIC_LIB    = $(BUILD_DIR)/lib/libeq$(MODULE).a
FAT_DYNAMIC_LIB   = $(BUILD_DIR)/lib/libeq$(MODULE).$(DSO_SUFFIX)
INSTALL_LIBS     ?= $(wildcard $(BUILD_DIR)/$(VARIANT)/lib/*.a \
                               $(BUILD_DIR)/$(VARIANT)/lib/*.$(DSO_SUFFIX))

ifdef VARIANT
THIN_STATIC_LIBS  = $(BUILD_DIR)/$(VARIANT)/lib/libeq$(MODULE).a
THIN_DYNAMIC_LIBS = $(BUILD_DIR)/$(VARIANT)/lib/libeq$(MODULE).$(DSO_SUFFIX)

else

THIN_STATIC_LIBS  = $(foreach V,$(VARIANTS),$(BUILD_DIR)/$(V)/lib/libeq$(MODULE).a)
THIN_DYNAMIC_LIBS = $(foreach V,$(VARIANTS),$(BUILD_DIR)/$(V)/lib/libeq$(MODULE).$(DSO_SUFFIX))
endif

# executable target
THIN_PROGRAMS     = $(foreach V,$(VARIANTS),$(PROGRAM).$(V))
FAT_PROGRAM       = $(PROGRAM)

FAT_SIMPLE_PROGRAMS  = $(CXXFILES:%.cpp=%)
THIN_SIMPLE_PROGRAMS = $(foreach V,$(VARIANTS),$(foreach P,$(FAT_SIMPLE_PROGRAMS),$(P).$(V)))
TESTS               ?= $(THIN_SIMPLE_PROGRAMS:%=testRun.%)

DYNAMIC_LIB       = $(THIN_DYNAMIC_LIBS)
STATIC_LIB        = $(THIN_STATIC_LIBS)
PROGRAMS          = $(THIN_PROGRAMS)
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
