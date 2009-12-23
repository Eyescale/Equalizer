
WINDOW_SYSTEM_LIBS += -L/usr/X11R6/lib -lX11 -lGL
ARCHFLAGS          ?= -m32 -march=pentium   # need at leat 486 for atomics

CUDA_LIBRARY_PATH ?= /usr/local/cuda/lib

# Check presence of CUDA
ifeq ($(wildcard $(CUDA_LIBRARY_PATH)/libcuda.so), $(CUDA_LIBRARY_PATH)/libcuda.so)
    DEFFLAGS += -DEQ_USE_CUDA
endif

