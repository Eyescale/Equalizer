
BUILD_FAT  = 1

ifeq ($(findstring 9.0, $(RELARCH)),9.0)
  LEOPARD = 1
  WINDOW_SYSTEM ?= AGL  # GLX is broken in build 559
endif

ifdef LEOPARD
  VARIANTS ?= i386
  CXXFLAGS += -DLEOPARD
  USE_SDK   = 1
else
  ifeq ($(findstring i386, $(SUBARCH)),i386)
    VARIANTS ?= i386 ppc
  else
    VARIANTS ?= ppc
  endif
endif

ifdef VARIANT
  CXXFLAGS    += -arch $(VARIANT) -Wno-unknown-pragmas
  DSO_LDFLAGS += -arch $(VARIANT)
endif


ifdef USE_SDK
  SDK         ?= /Developer/SDKs/MacOSX10.5.sdk
  CXXFLAGS    += -isysroot $(SDK)
  LDFLAGS     += -Wl,-syslibroot $(SDK)
endif

DSO_LDFLAGS        += -dynamiclib
DSO_SUFFIX          = dylib
WINDOW_SYSTEM      ?= GLX AGL

ifeq ($(findstring GLX, $(WINDOW_SYSTEM)),GLX)
  WINDOW_SYSTEM_LIBS += -L/usr/X11R6/lib -lX11 -lGL
  WINDOW_SYSTEM_INCS += -I/usr/X11R6/include
endif
ifeq ($(findstring AGL, $(WINDOW_SYSTEM)),AGL)
  WINDOW_SYSTEM_LIBS += -framework AGL -framework OpenGL -framework Carbon
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
