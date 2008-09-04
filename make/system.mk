
# location for top-level directory
ifndef TOP
  TOP := .
endif

# os-specific settings
ARCH    ?= $(subst .,_,$(subst -,_,$(shell uname)))
SUBARCH ?= $(shell uname -m)
RELARCH ?= $(shell uname -r)

include $(TOP)/make/$(ARCH).mk
-include $(TOP)/make/local.mk

DOXYGEN         ?= Doxygen
FLEX            ?= flex
BISON           ?= bison
PC_LIBRARY_PATH ?= /opt/paracomp/lib64

# What to pass down to sub-makes
export CFLAGS
export CXXFLAGS
export LDFLAGS
export LD
export PC_LIBRARY_PATH

# helper variables for directory-dependent stuff
SUBDIR    ?= "."
SUBDIRTOP := ../$(TOP)
DEPTH     := $(subst ../,--,$(TOP))
DEPTH     := $(subst .,-->,$(DEPTH))

# general variables, targets, etc.
BUILD_DIR_BASE  = build/$(ARCH)
BUILD_DIR       = $(TOP)/$(BUILD_DIR_BASE)
LIBRARY_DIR     = $(BUILD_DIR)/lib
INCLUDEDIRS     = -I$(BUILD_DIR)/include
LINKDIRS        = -L$(LIBRARY_DIR)
BIN_DIR        ?= $(BUILD_DIR)/bin

WINDOW_SYSTEM_DEFINES = $(foreach WS,$(WINDOW_SYSTEM),-D$(WS))
DEP_CXX        ?= $(CXX)
DEFFLAGS       += -D$(ARCH) $(WINDOW_SYSTEM_DEFINES) -DEQ_CHECK_THREADSAFETY \
                  -DGLEW_MX # -DEQ_ASYNC_TRANSMIT


ifeq (0,${MAKELEVEL}) # top-level invocation - one-time declarations below

LD        = $(CXX) # use c++ compiler for linking

ifneq ($(findstring -g, $(CXXFLAGS)),-g)
    DEFFLAGS       += -DNDEBUG
ifneq ($(findstring -O, $(CXXFLAGS)),-O)
    CXXFLAGS       += -O2
endif # -O
endif # -g

  CFLAGS         += $(DEFFLAGS)
  CXXFLAGS       += $(DEFFLAGS)

# ICC settings
ifeq ($(findstring icc, $(CXX)),icc)
    ICC_DIR    ?= /opt/intel/cc/10.1.014
    DEFFLAGS   += -DEQ_USE_OPENMP
    CXXFLAGS   += -openmp
    LDFLAGS    += -L$(ICC_DIR)/lib -lirc -lguide -limf -lsvml
    LD          = g++
endif # icc

# GCC settings
ifeq ($(findstring g++, $(CXX)),g++)
    CXXFLAGS += -Wall \
                -Wnon-virtual-dtor -Wsign-promo -Wshadow \
                -Wno-unknown-pragmas -Wno-unused-parameter -Wno-write-strings
ifdef USE_OPENMP
    DEFFLAGS += -DEQ_USE_OPENMP
    CXXFLAGS += -fopenmp
    LDFLAGS  += -lgomp
endif
endif # g++

# Paracomp settings
ifeq ($(wildcard $(PC_LIBRARY_PATH)), $(PC_LIBRARY_PATH))
    DEFFLAGS += -DEQ_USE_PARACOMP
    CXXFLAGS += -I$(PC_LIBRARY_PATH)/../include
    LDFLAGS  += -L$(PC_LIBRARY_PATH) -lpcstub
endif

endif # top-level

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
OBJECT_SUFFIX   = $(ARCH)

OBJECTS         = $(CXXFILES:%.cpp=$(OBJECT_DIR)/%.$(OBJECT_SUFFIX).o) \
		  $(CFILES:%.c=$(OBJECT_DIR)/%.$(OBJECT_SUFFIX).o)
DEPENDENCIES    = $(OBJECTS:%=%.d) $(SIMPLE_PROGRAMS:%=%.d)
#  PCHEADERS     = $(HEADER_SRC:%=$(OBJECT_DIR)/%.gch)

# library variables
LIBRARY           = $(DYNAMIC_LIB)

STATIC_LIB  = $(LIBRARY_DIR)/lib$(MODULE).a)
DYNAMIC_LIB = $(LIBRARY_DIR)/lib$(MODULE).$(DSO_SUFFIX)

# executable target
PROGRAMS    ?= $(PROGRAM_EXE)
PROGRAM_EXE  = $(BIN_DIR)/$(PROGRAM)
PROGRAM_APP  = $(BIN_DIR)/$(PROGRAM).app/Contents/MacOS/$(PROGRAM)

SIMPLE_PROGRAMS  = $(SIMPLE_CXXFILES:%.cpp=$(BIN_DIR)/%)
TESTS           ?= $(SIMPLE_PROGRAMS:%=%.testOk)

# install variables
INSTALL_CMD     = $(TOP)/install.sh
INSTALL_FILES   = $(TOP)/install.files
