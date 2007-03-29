
VARIANTS    ?= 64

ifdef VARIANT
  CXXFLAGS    += -m$(VARIANT) -fPIC
  DSO_LDFLAGS += -m$(VARIANT)
endif

ifeq ($(findstring 32, $(VARIANT)),32)
  LDFLAGS            += -m32
  WINDOW_SYSTEM_LIBS += -L/usr/X11R6/lib -lX11 -lGL
  INSTALL_LIB_DIR     = $(INSTALL_DIR)/lib
else
  WINDOW_SYSTEM_LIBS += -L/usr/X11R6/lib64 -lX11 -lGL
  INSTALL_LIB_DIR     = $(INSTALL_DIR)/lib64
endif

