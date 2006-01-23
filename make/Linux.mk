
VARIANTS    = 64

ifdef VARIANT
  CXXFLAGS    += -m$(VARIANT) -fPIC
  DSO_LDFLAGS += -m$(VARIANT)
endif

DSO_LDFLAGS        += -shared
DSO_SUFFIX          = so
WINDOW_SYSTEM      += GLX

ifeq ($(findstring GLX, $(WINDOW_SYSTEM)),GLX)
  WINDOW_SYSTEM_LIBS += -L/usr/X11R6/lib$(VARIANT1) -lX11 -lGL
  WINDOW_SYSTEM_INCS += -I/usr/X11R6/include
endif

AR           = ld
ARFLAGS      = -r
