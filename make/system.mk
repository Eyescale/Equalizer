
# location for top-level directory
ifndef TOP
  TOP := .
endif

# os-specific settings
ARCH    ?= $(subst .,_,$(subst -,_,$(shell uname)))
SUBARCH ?= $(shell uname -m)
RELARCH ?= $(shell uname -r)

-include $(TOP)/make/local.mk
include $(TOP)/make/$(ARCH).mk

DOXYGEN         ?= Doxygen
FLEX            ?= flex
BISON           ?= bison
PC_LIBRARY_PATH ?= /opt/paracomp/lib64

CUDA_LIBRARY_PATH ?= /usr/local/cuda/lib
CUDA_INCLUDE_PATH ?= /usr/local/cuda/include

# What to pass down to sub-makes
export CFLAGS
export CXXFLAGS
export LDFLAGS
export LD
export LD_PATH
export PC_LIBRARY_PATH
export CUDA_LIBRARY_PATH
export CUDA_INCLUDE_PATH
export BOOST_LIBRARY_PATH
export BOOST_INCLUDE_PATH

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
DEFFLAGS       += -D$(ARCH) $(WINDOW_SYSTEM_DEFINES) -DGLEW_MX
	 # -DEQ_USE_DEPRECATED

SHADERS_PARSER  = $(TOP)/make/stringify.pl

ifeq (0,${MAKELEVEL}) # top-level invocation - one-time declarations below

LD        = $(CXX) # use c++ compiler for linking
LD_PATH   = $(PWD)/$(LIBRARY_DIR)

ifeq ($(findstring -O, $(CXXFLAGS)),-O) # assume release build...
ifeq ($(findstring -g, $(CXXFLAGS)),-g) # ... unless -g was specified
    CXXFLAGS       += -Werror
else
    CXXFLAGS       += -Werror -Wuninitialized
    DEFFLAGS       += -DNDEBUG
endif # -g
else  # debug build
    CXXFLAGS       += -g -Werror
endif # -O

ifndef CFLAGS
  CFLAGS         := $(CXXFLAGS)
endif

  CFLAGS         += $(DEFFLAGS)
  CXXFLAGS       += $(DEFFLAGS)

# ICC settings
ifeq ($(findstring icc, $(CXX)),icc)
    ICC_DIR    ?= /opt/intel/Compiler/11.1/064/
    DEFFLAGS   += -DEQ_USE_OPENMP
    CXXFLAGS   += -openmp -Wno-deprecated -Wno-overloaded-virtual -diag-disable 654
    LDFLAGS    += -L$(ICC_DIR)/lib/intel64 -lirc -lguide -limf -lsvml -lpthread
    LD_PATH    = $(PWD)/$(LIBRARY_DIR):$(ICC_DIR)/lib/intel64
    LD          = g++
endif # icc

# GCC settings
ifeq ($(findstring g++, $(CXX)),g++)
    CXXFLAGS += -Wall \
                -Wnon-virtual-dtor -Wsign-promo -Wshadow -Winit-self \
                -Wno-unknown-pragmas -Wno-unused-parameter -Wno-write-strings
ifdef USE_OPENMP
    DEFFLAGS += -DEQ_USE_OPENMP
    CXXFLAGS += -fopenmp
    LDFLAGS  += -lgomp
endif
endif # g++

# CUDA settings
ifeq ($(findstring -DEQ_USE_CUDA, $(DEFFLAGS)), -DEQ_USE_CUDA)
    CXXFLAGS += -I$(CUDA_INCLUDE_PATH)
    LDFLAGS  += -L$(CUDA_LIBRARY_PATH) -lcuda -lcudart	
    LD_PATH  := "$(LD_PATH):$(CUDA_LIBRARY_PATH)"
endif

# BOOST settings
# Check presence of BOOST
ifeq ($(wildcard $(BOOST_INCLUDE_PATH)/boost/asio), $(BOOST_INCLUDE_PATH)/boost/asio)
    DEFFLAGS += -DEQ_USE_BOOST

    CXXFLAGS += -isystem $(BOOST_INCLUDE_PATH)
    LDFLAGS  += -L$(BOOST_LIBRARY_PATH) -lboost_system
    LD_PATH  := "$(LD_PATH):$(BOOST_LIBRARY_PATH)"
endif

# Paracomp settings
ifeq ($(findstring x86_64, $(SUBARCH)), x86_64)
ifeq ($(wildcard $(PC_LIBRARY_PATH)), $(PC_LIBRARY_PATH))
    DEFFLAGS += -DEQ_USE_PARACOMP
    DEFFLAGS += -DEQ_USE_PARACOMP_BLEND
#    DEFFLAGS += -DEQ_USE_PARACOMP_DEPTH
    CXXFLAGS += -I$(PC_LIBRARY_PATH)/../include
    LDFLAGS  += -L$(PC_LIBRARY_PATH) -lpcstub
endif
endif

endif # top-level

#build mode
ifeq ($(findstring -g, $(CXXFLAGS)),-g)
    BUILD_MODE     = Debug
else
    BUILD_MODE     = Release
endif # -g

# defines
CXX_DEFINES_TMP   = $(sort $(filter -D%,$(CXXFLAGS))) \
                    $(sort $(filter -D%,$(CFLAGS)))
CXX_DEFINES       = $(CXX_DEFINES_TMP:NDEBUG=)
CXX_DEFINES_FILE ?= lib/base/defines.h
CXX_DEFINES_TXT   = $(CXX_DEFINES:-D%= %)

# include file variables
INCLUDE_BASE    = include/$(MODULE)
INCLUDE_DIR     = $(BUILD_DIR)/$(INCLUDE_BASE)
HEADERS         = $(HEADER_SRC:%=$(INCLUDE_DIR)/%)

# share files
SHARE_DIR       = $(BUILD_DIR)/share/Equalizer

# source code variables
SIMPLE_CXXFILES = $(wildcard *.cpp)
OBJECT_DIR      = $(TOP)/obj/$(BUILD_MODE)/$(SUBDIR)
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
TESTS           ?= $(SIMPLE_PROGRAMS:%=%.testOK)

# install variables
INSTALL_CMD     = $(TOP)/install.sh
INSTALL_FILES   = $(TOP)/install.files
