
include $(TOP)/make/system.mk

NVCC ?= $(CUDA_BIN_PATH)/nvcc
CUFLAGS = --compile

PROGRAM = eqNBody

CXXFILES = \
	channel.cpp \
	client.cpp \
	config.cpp \
	configEvent.cpp \
	controller.cpp \
	frameData.cpp \
	initData.cpp \
	main.cpp \
	node.cpp \
	pipe.cpp \
	render_particles.cpp \
	sharedData.cpp \
	sharedDataProxy.cpp \
	window.cpp

CXXFLAGS += -I../include $(WINDOW_SYSTEM_INCS) -I/usr/local/cuda/include
LDFLAGS  += -lEqualizer -lpthread $(WINDOW_SYSTEM_LIBS)  nbody.o \
	    -L/usr/local/cuda/lib -lcuda -lcudart

ifeq "${ARCH}" "Linux"
  LDFLAGS += -lrt
endif

TARGETS = cuda $(PROGRAMS)

include $(TOP)/make/rules.mk

cuda: nbody.o nbody_kernel.o

nbody.o: nbody.cu
	$(NVCC) $(CUFLAGS) nbody.cu -o nbody.o

nbody_kernel.o: nbody_kernel.cu
	$(NVCC) $(CUFLAGS) nbody_kernel.cu -o nbody_kernel.o
