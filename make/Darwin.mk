
#USE_OPENMP      = 1
#EQ_USE_MAGELLAN = 1

CC ?= /usr/bin/gcc-4.2
CXX ?= /usr/bin/g++-4.2

DSO_SUFFIX       = dylib
DSO_LDFLAGS     += -dynamiclib

AR               = libtool
ARFLAGS          = -static

PC_LIBRARY_PATH   ?= /opt/paracomp/lib
BOOST_LIBRARY_PATH ?= /opt/local/lib
BOOST_INCLUDE_PATH ?= /opt/local/include

ifeq ($(findstring 10., $(RELARCH)),10.) # 10.6
  DARWIN_RELEASE = 10.6
endif # SNOWLEOPARD

ifeq ($(findstring 9., $(RELARCH)),9.) # 10.5
  DARWIN_RELEASE = 10.5
endif # LEOPARD

include $(TOP)/make/$(ARCH).$(DARWIN_RELEASE).mk

ifeq (0,${MAKELEVEL})
  CXXFLAGS        += -Wextra
endif

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

ifeq ($(findstring GLX, $(WINDOW_SYSTEM)),GLX)
  WINDOW_SYSTEM_LIBS += -L/usr/X11R6/lib -lX11 -lGL
  WINDOW_SYSTEM_INCS += -I/usr/X11R6/include
  CFLAGS += -DGLEW_APPLE_GLX
endif # GLX

ifeq ($(findstring AGL, $(WINDOW_SYSTEM)),AGL)
  WINDOW_SYSTEM_LIBS += -framework AGL -framework OpenGL -framework Carbon
  PROGRAMS            = $(PROGRAM_EXE) $(PROGRAM_APP)
endif

ifdef EQ_USE_MAGELLAN
  WINDOW_SYSTEM_LIBS += -framework 3DconnexionClient
  CXXFLAGS           += -DEQ_USE_MAGELLAN
endif
