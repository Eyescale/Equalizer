
# BUILD_32BIT = 1

ifdef BUILD_32BIT
  CXXFLAGS += -m32 -fPIC
  CFLAGS   += -m32 -fPIC
  LDFLAGS  += -m32
  WINDOW_SYSTEM_LIBS += -L/usr/X11R6/lib -lX11 -lGL
  INSTALL_LIB_DIR     = $(INSTALL_DIR)/lib
else
  CXXFLAGS += -m64 -fPIC
  CFLAGS   += -m64 -fPIC
  LDFLAGS  += -m64
  WINDOW_SYSTEM_LIBS += -L/usr/X11R6/lib64 -lX11 -lGL
  INSTALL_LIB_DIR     = $(INSTALL_DIR)/lib64
endif

