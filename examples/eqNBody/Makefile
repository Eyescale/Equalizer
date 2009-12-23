
include $(TOP)/make/system.mk

NVCC = nvcc
CUFLAGS = --compile

PROGRAM = eqNBody

CXXFILES = \
	channel.cpp \
	client.cpp \
	config.cpp \
	configEvent.cpp \
	controller.cpp \
	dataProxy.cpp \
	frameData.cpp \
	initData.cpp \
	main.cpp \
	node.cpp \
	pipe.cpp \
	render_particles.cpp \
	sharedData.cpp \
	window.cpp

CXXFLAGS += -I../include $(WINDOW_SYSTEM_INCS) -I/usr/local/cuda/include
LDFLAGS  += -leq -lpthread $(WINDOW_SYSTEM_LIBS)  nbody.o -L/usr/local/cuda/lib -lcuda -lcudart #-deviceemu

ifeq "${ARCH}" "Linux"
  LDFLAGS += -lrt
ifeq "${SUBARCH}" "ia64"
    LDFLAGS += -luuid
endif
endif

TARGETS = cuda $(PROGRAMS)

include $(TOP)/make/rules.mk

cuda: nbody.o nbody_kernel.o

nbody.o: nbody.cu
	$(NVCC) $(CUFLAGS) nbody.cu -o nbody.o

nbody_kernel.o: nbody_kernel.cu
	$(NVCC) $(CUFLAGS) nbody_kernel.cu -o nbody_kernel.o
