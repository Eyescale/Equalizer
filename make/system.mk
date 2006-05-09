
# location for top-level directory
ifndef TOP
  TOP := .
endif

SUBDIR    ?= "src"
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
INSTALL_LIBDIR ?= $(INSTALL_DIR)/lib$(VARIANT)
BUILD_DIR       = $(TOP)/build/$(ARCH)
EXTRAS_DIR      = $(TOP)/extras
LIBRARY_DIR     = $(BUILD_DIR)/$(VARIANT)/lib

WINDOW_SYSTEM_DEFINES = $(foreach WS,$(WINDOW_SYSTEM),-D$(WS))
CXXFLAGS       += -D$(ARCH) $(WINDOW_SYSTEM_DEFINES) -DCHECK_THREADSAFETY
INT_CXXFLAGS   += -I$(OBJECT_DIR) -I$(BUILD_DIR)/include -I$(EXTRAS_DIR)
LDFLAGS        += -L$(LIBRARY_DIR)
DEP_CXX        ?= $(CXX)
DOXYGEN        ?= Doxygen

# defines
CXX_DEFINES      = $(filter -D%,$(CXXFLAGS) $(INT_CXXFLAGS))
CXX_DEFINES_FILE = lib/base/defines.h
CXX_DEFINES_TXT  = $(CXX_DEFINES:-D%= %)

# header file variables
HEADER_DIR      = $(BUILD_DIR)/include/eq/$(MODULE)
HEADERS         = $(HEADER_SRC:%=$(HEADER_DIR)/%)

# source code variables
CXXFILES        = $(wildcard *.cpp)
OBJECT_DIR      = obj/$(ARCH)/$(VARIANT)

ifndef VARIANT
  OBJECTS       = foo
else
  OBJECTS       = $(SOURCES:%.cpp=$(OBJECT_DIR)/%.o)
#  PCHEADERS     = $(HEADER_SRC:%=$(OBJECT_DIR)/%.gch)
endif

# library variables
LIBRARY           = $(DYNAMIC_LIB)
FAT_STATIC_LIB    = $(BUILD_DIR)/lib/libeq$(MODULE).a
FAT_DYNAMIC_LIB   = $(BUILD_DIR)/lib/libeq$(MODULE).$(DSO_SUFFIX)
INSTALL_LIBS     ?= $(wildcard $(BUILD_DIR)/lib/$(VARIANT)/*.a \
                               $(BUILD_DIR)/lib/$(VARIANT)/*.$(DSO_SUFFIX))

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

DEPENDENCIES   ?= $(OBJECTS:%.o=%.d)

