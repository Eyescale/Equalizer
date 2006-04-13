
VARIANTS = ppc

CXXFLAGS += #-DEQLOADER

ifdef VARIANT
  CXXFLAGS    += -arch $(VARIANT)
  DSO_LDFLAGS += -arch $(VARIANT)
endif

DSO_LDFLAGS        += -dynamiclib -undefined dynamic_lookup
DSO_SUFFIX          = dylib
#WINDOW_SYSTEM      += GLX CGL
WINDOW_SYSTEM      += GLX

export MACOSX_DEPLOYMENT_TARGET=10.3

ifeq ($(findstring GLX, $(WINDOW_SYSTEM)),GLX)
  LDFLAGS  += -L/usr/X11R6/lib -lX11 -lGL
  CXXFLAGS += -I/usr/X11R6/include
endif
ifeq ($(findstring CGL, $(WINDOW_SYSTEM)),CGL)
# TODO
endif

AR           = libtool
ARFLAGS      = -static
