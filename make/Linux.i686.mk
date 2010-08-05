
WINDOW_SYSTEM_LIBS += -L/usr/X11R6/lib -lX11 -lGL
ARCHFLAGS          ?= -m32 -march=pentium   # need at leat 486 for atomics

