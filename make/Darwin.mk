
BUILD_FAT = 1

ifeq ($(findstring i386, $(SUBARCH)),i386)
  VARIANTS ?= ppc i386
else
  VARIANTS ?= ppc
endif

ifdef VARIANT
  CXXFLAGS    += -arch $(VARIANT) -Wno-unknown-pragmas
  DSO_LDFLAGS += -arch $(VARIANT)
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

# default bison on Darwin is too old, use fink version. 
# Change to /opt/local/bin/bison if you are using DarwinPorts
BISON        = /sw/bin/bison