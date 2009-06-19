
#USE_OPENMP       = 1
#EQ_USE_MAGELLAN = 1

DSO_SUFFIX       = dylib
DSO_LDFLAGS     += -dynamiclib
WINDOW_SYSTEM   ?= GLX AGL

AR               = libtool
ARFLAGS          = -static
PC_LIBRARY_PATH ?= /opt/paracomp/lib

ifeq (0,${MAKELEVEL})
  CXXFLAGS        += -Wextra
endif

ifeq ($(findstring 9., $(RELARCH)),9.)
  LEOPARD       = 1
  CXXFLAGS     += -DLEOPARD
  AGL_OR_64BIT  = AGL

ifeq ($(findstring icc, $(CXX)),icc)
  ARCHFLAGS     ?= -m32
endif

ifeq ($(findstring 64BIT, $(AGL_OR_64BIT)), 64BIT)
  ARCHFLAGS     ?= -arch i386 -arch ppc -arch x86_64 -arch ppc64
  WINDOW_SYSTEM  = GLX
endif # 64BIT
endif # LEOPARD

ifeq ($(findstring i386, $(SUBARCH)), i386)
  ARCHFLAGS ?= -arch i386 -arch ppc
else
  ARCHFLAGS ?= -arch ppc
endif

ifeq ($(findstring GLX, $(WINDOW_SYSTEM)),GLX)
  WINDOW_SYSTEM_LIBS += -L/usr/X11R6/lib -lX11 -lGL
  WINDOW_SYSTEM_INCS += -I/usr/X11R6/include
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