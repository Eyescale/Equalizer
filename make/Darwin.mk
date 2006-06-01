
BUILD_FAT = 1

ifeq ($(findstring i386, $(SUBARCH)),i386)
  VARIANTS = ppc i386
else
  VARIANTS = ppc
endif

ifdef VARIANT
  CXXFLAGS    += -arch $(VARIANT)
  DSO_LDFLAGS += -arch $(VARIANT)
endif

# needed for '-undefined dynamic_lookup'
export MACOSX_DEPLOYMENT_TARGET=10.3

DSO_LDFLAGS        += -dynamiclib -undefined dynamic_lookup
DSO_SUFFIX          = dylib
#WINDOW_SYSTEM      += GLX CGL
WINDOW_SYSTEM      += GLX

ifeq ($(findstring GLX, $(WINDOW_SYSTEM)),GLX)
  LDFLAGS  += -L/usr/X11R6/lib -lX11 -lGL
  CXXFLAGS += -I/usr/X11R6/include
endif
ifeq ($(findstring CGL, $(WINDOW_SYSTEM)),CGL)
# TODO
endif

BISON        = /sw/bin/bison  # installed bison is to old on Darwin
AR           = libtool
ARFLAGS      = -static
