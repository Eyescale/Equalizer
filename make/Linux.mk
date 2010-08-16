
# common Linux settings

USE_OPENMP          = 1
DSO_LDFLAGS        += -shared
DSO_SUFFIX          = so
WINDOW_SYSTEM      ?= GLX
WINDOW_SYSTEM_INCS += -I/usr/X11R6/include
AR                  = ld
ARFLAGS             = -r
CXXFLAGS           += -Wno-deprecated
BOOST_INCLUDE_PATH ?= /usr/include
BOOST_LIBRARY_PATH ?= /usr/lib
LD_PATH := /usr/lib/nvidia-current:$(LD_PATH)

# enable internal compiler error WAR
ifndef BUILD_MODE
release: GCCWAR = 1
endif

# SUBARCH-specific settings
include $(TOP)/make/$(ARCH).$(SUBARCH).mk
