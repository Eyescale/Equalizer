
VARIANTS    = ppc

ifdef VARIANT
  CXXFLAGS    += -arch $(VARIANT)
  DSO_LDFLAGS += -arch $(VARIANT)
endif

# export MACOSX_DEPLOYMENT_TARGET = 10.3

DSO_LDFLAGS        += -dynamiclib -flat_namespace -undefined warning
DSO_SUFFIX          = dylib
#WINDOW_SYSTEM      += GLX CGL
WINDOW_SYSTEM      += GLX

ifeq ($(findstring GLX, $(WINDOW_SYSTEM)),GLX)
  WINDOW_SYSTEM_LIBS += -L/usr/X11R6/lib -lX11 -lGL
  WINDOW_SYSTEM_INCS += -I/usr/X11R6/include
endif
ifeq ($(findstring CGL, $(WINDOW_SYSTEM)),CGL)
# TODO
endif

AR           = libtool
ARFLAGS      = -static
