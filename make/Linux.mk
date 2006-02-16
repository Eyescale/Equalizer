
VARIANTS    = 64

ifdef VARIANT
  CXXFLAGS    += -m$(VARIANT) -fPIC
  DSO_LDFLAGS += -m$(VARIANT)
else
  CXXFLAGS    += -m$(VARIANT1) -fPIC
  DSO_LDFLAGS += -m$(VARIANT1)
endif

DSO_LDFLAGS        += -shared
DSO_SUFFIX          = so
WINDOW_SYSTEM      += GLX

ifeq ($(findstring 32, $(VARIANT)),32)
  LDFLAGS            += -melf_i386
endif

ifeq ($(findstring 32, $(VARIANT1)),32)
  WINDOW_SYSTEM_LIBS += -L/usr/X11R6/lib -lX11 -lGL
else
  WINDOW_SYSTEM_LIBS += -L/usr/X11R6/lib$(VARIANT1) -lX11 -lGL
endif

WINDOW_SYSTEM_INCS += -I/usr/X11R6/include

AR           = ld
ARFLAGS      = -r
