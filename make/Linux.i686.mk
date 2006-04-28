
VARIANTS    = i686

DSO_LDFLAGS        += -shared -luuid
DSO_SUFFIX          = so
WINDOW_SYSTEM      += GLX
WINDOW_SYSTEM_LIBS += -L/usr/X11R6/lib -lX11 -lGL
WINDOW_SYSTEM_INCS += -I/usr/X11R6/include

AR                  = ld
ARFLAGS             = -r
