
# common Linux settings

USE_OPENMP          = 1
DSO_LDFLAGS        += -shared
DSO_SUFFIX          = so
WINDOW_SYSTEM      ?= GLX
WINDOW_SYSTEM_INCS += -I/usr/X11R6/include
AR                  = ld
ARFLAGS             = -r
CXXFLAGS           += -Wno-deprecated
CUDA_INCLUDE_PATH  ?= /usr/local/cuda/include
BOOST_INCLUDE_PATH ?= /usr/include

# SUBARCH-specific settings
include $(TOP)/make/$(ARCH).$(SUBARCH).mk

# Check presence of CUDA
ifeq ($(wildcard $(CUDA_LIBRARY_PATH)/libcuda.so), $(CUDA_LIBRARY_PATH)/libcuda.so)
    DEFFLAGS += -DEQ_USE_CUDA
endif

