
#USE_OPENMP       = 1
#EQ_USE_MAGELLAN = 1

DSO_SUFFIX       = dylib
DSO_LDFLAGS     += -dynamiclib
WINDOW_SYSTEM   ?= AGL 

AR               = libtool
ARFLAGS          = -static

PC_LIBRARY_PATH   ?= /opt/paracomp/lib

CUDA_LIBRARY_PATH ?= /usr/local/cuda/lib
CUDA_INCLUDE_PATH ?= /usr/local/cuda/include

# Check presence of CUDA
ifeq ($(wildcard $(CUDA_LIBRARY_PATH)/libcuda.dylib), $(CUDA_LIBRARY_PATH)/libcuda.dylib)
    DEFFLAGS += -DEQ_USE_CUDA
    CUDA      = 1
endif

ifeq (0,${MAKELEVEL})
  CXXFLAGS        += -Wextra
endif

ifeq ($(findstring 10., $(RELARCH)),10.) # 10.6
  LEOPARD       = 1
endif # SNOWLEOPARD

ifeq ($(findstring 9., $(RELARCH)),9.) # 10.5
  LEOPARD       = 1
endif # LEOPARD

ifdef LEOPARD
  CXXFLAGS     += -DLEOPARD

ifeq ($(findstring icc, $(CXX)),icc)
  ARCHFLAGS     ?= -m32
endif # icc

ifeq ($(findstring AGL, $(WINDOW_SYSTEM)), AGL)
  AGL_32BIT_ONLY = 1
endif

ifeq ($(findstring i386, $(SUBARCH)), i386)
ifdef AGL_32BIT_ONLY
ifdef CUDA
  ARCHFLAGS ?= -arch i386
else
  ARCHFLAGS ?= -arch i386 -arch ppc
endif
else
ifdef CUDA
  ARCHFLAGS ?= -arch i386 -arch x86_64 
else
  ARCHFLAGS ?= -arch i386 -arch ppc -arch x86_64 -arch ppc64
endif
endif # 64BIT
else
ifdef AGL_32BIT_ONLY
  ARCHFLAGS ?= -arch ppc
else
  ARCHFLAGS ?= -arch ppc -arch ppc64
endif
endif

endif # LEOPARD

ifeq ($(findstring GLX, $(WINDOW_SYSTEM)),GLX)
  WINDOW_SYSTEM_LIBS += -L/usr/X11R6/lib -lX11 -lGL
  WINDOW_SYSTEM_INCS += -I/usr/X11R6/include
  CFLAGS += -DGLEW_APPLE_GLX
ifdef LEOPARD
    # war to broken libGL in Leopard
    WINDOW_SYSTEM_LIBS += -dylib_file /System/Library/Frameworks/OpenGL.framework/Versions/A/Libraries/libGL.dylib:/System/Library/Frameworks/OpenGL.framework/Versions/A/Libraries/libGL.dylib
endif # LEOPARD
endif # GLX

ifeq ($(findstring AGL, $(WINDOW_SYSTEM)),AGL)
  WINDOW_SYSTEM_LIBS += -framework AGL -framework OpenGL -framework Carbon
  PROGRAMS            = $(PROGRAM_EXE) $(PROGRAM_APP)
endif

ifdef LEOPARD
  BISON        = bison
else
 # default bison on Tiger and earlier is too old, use fink or macports version
 BISONS       = $(wildcard /opt/local/bin/bison /sw/bin/bison )
 BISON        = $(word 1, $(BISONS))
endif

ifdef EQ_USE_MAGELLAN
  WINDOW_SYSTEM_LIBS += -framework 3DconnexionClient
  CXXFLAGS     += -DEQ_USE_MAGELLAN
endif
