
VARIANTS    = 32 64

ifdef VARIANT
  CXXFLAGS    += -m$(VARIANT) -fPIC
  DSO_LDFLAGS += -m$(VARIANT)
endif

DSO_LDFLAGS        += -shared -luuid
DSO_SUFFIX          = so
WINDOW_SYSTEM      += GLX

ifeq ($(findstring 32, $(VARIANT)),32)
  LDFLAGS            += -m32
  WINDOW_SYSTEM_LIBS += -L/usr/X11R6/lib -lX11 -lGL
else
  WINDOW_SYSTEM_LIBS += -L/usr/X11R6/lib64 -lX11 -lGL
endif

WINDOW_SYSTEM_INCS += -I/usr/X11R6/include

AR           = ld
ARFLAGS      = -r
