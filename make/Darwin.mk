
BUILD_FAT  = 1

ifeq ($(findstring 9.0, $(RELARCH)),9.0)
  LEOPARD       = 1
  CXXFLAGS     += -DLEOPARD
  AGL_OR_64BIT  = AGL

ifeq ($(findstring 64BIT, $(AGL_OR_64BIT)), 64BIT)
  VARIANTS      ?= i386 ppc x86_64 ppc64
  WINDOW_SYSTEM  = GLX
endif # 64BIT
endif # LEOPARD

ifeq ($(findstring i386, $(SUBARCH)), i386)
  VARIANTS ?= i386 ppc
else
  VARIANTS ?= ppc
endif

ifeq ($(findstring g++-4.2, $(CXX)),g++-4.2)
  USE_OPENMP = 1
endif # g++ 4.2

ifdef VARIANT
  CXXFLAGS    += -arch $(VARIANT)
  DSO_LDFLAGS += -arch $(VARIANT)
endif

DSO_LDFLAGS        += -dynamiclib
DSO_SUFFIX          = dylib
WINDOW_SYSTEM      ?= GLX AGL

ifeq ($(findstring GLX, $(WINDOW_SYSTEM)),GLX)
  WINDOW_SYSTEM_LIBS += -L/usr/X11R6/lib -lX11 -lGL
  WINDOW_SYSTEM_INCS += -I/usr/X11R6/include
ifdef LEOPARD
  # war to broken libGL in betas
  WINDOW_SYSTEM_LIBS += -dylib_file /System/Library/Frameworks/OpenGL.framework/Versions/A/Libraries/libGL.dylib:/System/Library/Frameworks/OpenGL.framework/Versions/A/Libraries/libGL.dylib
endif # LEOPARD
endif # GLX

ifeq ($(findstring AGL, $(WINDOW_SYSTEM)),AGL)
  WINDOW_SYSTEM_LIBS += -framework AGL -framework OpenGL -framework Carbon
  PROGRAMS           += $(APP_PROGRAM)
endif

AR           = libtool
ARFLAGS      = -static

ifdef LEOPARD
  BISON        = bison
else
 # default bison on Tiger and earlier is too old, use fink or macports version
 BISONS       = $(wildcard /opt/local/bin/bison /sw/bin/bison )
 BISON        = $(word 1, $(BISONS))
endif
